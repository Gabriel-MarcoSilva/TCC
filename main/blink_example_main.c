#include <stdio.h>
#include <string.h>
#include <stdlib.h> // Para malloc e free
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include "esp_netif.h"
#include <esp_http_server.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

// --- DEFINIÇÕES DE PINO E WIFI ---
#define EXAMPLE_ESP_WIFI_SSID      "HOME"
#define EXAMPLE_ESP_WIFI_PASS      "2019.1jgj"
#define EXAMPLE_ESP_MAXIMUM_RETRY  10
#define LED_PIN GPIO_NUM_2

// Motores Vibracall
#define MOTOR_PIN_1 GPIO_NUM_18      // Motor 1 (L)
#define MOTOR_PIN_2 GPIO_NUM_19      // Motor 2 (R)

static const char *TAG = "ESP_SERVER";
static int s_retry_num = 0;
static httpd_handle_t server = NULL;

// Variável global para rastrear a Task de vibração em execução
static TaskHandle_t vibration_task_handle = NULL;

// --- ESTRUTURA PARA DADOS DA TASK ---
typedef struct {
    char padrao[32];
    bool motor_1_on;
    bool motor_2_on;
} blink_task_params_t;

// ------------------- LED -------------------
void led_init() {
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);
    ESP_LOGI(TAG, "LED inicializado.");
}

// ------------------- MOTOR (PWM) -------------------
// Definições PWM para o Motor 1
#define MOTOR_1_LEDC_CHANNEL LEDC_CHANNEL_0
#define MOTOR_1_LEDC_TIMER   LEDC_TIMER_0
// Definições PWM para o Motor 2
#define MOTOR_2_LEDC_CHANNEL LEDC_CHANNEL_1
#define MOTOR_2_LEDC_TIMER   LEDC_TIMER_0 
#define MOTOR_LEDC_MODE    LEDC_LOW_SPEED_MODE
#define MOTOR_LEDC_FREQ_HZ 5000
#define MOTOR_LEDC_RES     LEDC_TIMER_8_BIT // 0-255

void motor_init() {
    // Configuração do Timer (comum para ambos os canais)
    ledc_timer_config_t timer_conf = {
        .speed_mode       = MOTOR_LEDC_MODE,
        .duty_resolution  = MOTOR_LEDC_RES,
        .timer_num        = MOTOR_1_LEDC_TIMER,
        .freq_hz          = MOTOR_LEDC_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    // Configuração do Canal 0 (Motor 1)
    ledc_channel_config_t channel_conf_1 = {
        .gpio_num   = MOTOR_PIN_1,
        .speed_mode = MOTOR_LEDC_MODE,
        .channel    = MOTOR_1_LEDC_CHANNEL,
        .timer_sel  = MOTOR_1_LEDC_TIMER,
        .duty       = 0,
        .hpoint     = 0
    };
    ledc_channel_config(&channel_conf_1);

    // Configuração do Canal 1 (Motor 2)
    ledc_channel_config_t channel_conf_2 = {
        .gpio_num   = MOTOR_PIN_2,
        .speed_mode = MOTOR_LEDC_MODE,
        .channel    = MOTOR_2_LEDC_CHANNEL,
        .timer_sel  = MOTOR_2_LEDC_TIMER,
        .duty       = 0, 
        .hpoint     = 0
    };
    ledc_channel_config(&channel_conf_2);

    ESP_LOGI(TAG, "Motores inicializados. M1:%d (Ch:%d), M2:%d (Ch:%d)",
             MOTOR_PIN_1, MOTOR_1_LEDC_CHANNEL, MOTOR_PIN_2, MOTOR_2_LEDC_CHANNEL);
}

// Função de controle de intensidade para o Motor 1
void motor_1_set_intensity(uint8_t intensity) {
    ledc_set_duty(MOTOR_LEDC_MODE, MOTOR_1_LEDC_CHANNEL, intensity);
    ledc_update_duty(MOTOR_LEDC_MODE, MOTOR_1_LEDC_CHANNEL);
}

// Função de controle de intensidade para o Motor 2
void motor_2_set_intensity(uint8_t intensity) {
    ledc_set_duty(MOTOR_LEDC_MODE, MOTOR_2_LEDC_CHANNEL, intensity);
    ledc_update_duty(MOTOR_LEDC_MODE, MOTOR_2_LEDC_CHANNEL);
}

// Função de desligar ambos
void motors_off() {
    motor_1_set_intensity(0);
    motor_2_set_intensity(0);
    gpio_set_level(LED_PIN, 0);
    ESP_LOGI(TAG, "Motores desligados.");
}

// ------------------- TASK DE PISCAR -------------------
static void blink_task(void *param) {
    blink_task_params_t *task_params = (blink_task_params_t *)param;
    char *padrao = task_params->padrao;
    bool motor_1_enabled = task_params->motor_1_on;
    bool motor_2_enabled = task_params->motor_2_on;
    
    // Loop principal para repetição contínua
    while(1) {
        ESP_LOGI(TAG, "Executando padrão: %s. Motor 1: %s, Motor 2: %s",
                 padrao, motor_1_enabled ? "ON" : "OFF", motor_2_enabled ? "ON" : "OFF");

        for (int i = 0; padrao[i] != '\0'; i++) {
            uint8_t intensity = 0;
            uint32_t pulse_duration = 0;

            if (padrao[i] == '1') {
                intensity = 255;
                pulse_duration = 133;
                gpio_set_level(LED_PIN, 1);
            } else if (padrao[i] == '0') {
                intensity = 255;
                pulse_duration = 400;
                gpio_set_level(LED_PIN, 1);
            } else {
                 continue;
            }

            // Aplica a intensidade apenas se o motor estiver habilitado
            motor_1_set_intensity(motor_1_enabled ? intensity : 0);
            motor_2_set_intensity(motor_2_enabled ? intensity : 0);
            
            // Duração do pulso
            vTaskDelay(pdMS_TO_TICKS(pulse_duration));

            // Desliga ambos os motores e LED para o GAP
            motors_off();
            vTaskDelay(pdMS_TO_TICKS(133)); // Duração do GAP
        }
        
        // Pausa de 2 segundos entre as repetições
        ESP_LOGI(TAG, "Pausa de 2s. Aguardando a próxima repetição...");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    // Esta parte do código só será alcançada se a task for encerrada externamente.
    motors_off();
    ESP_LOGI(TAG, "Task de vibração encerrada.");
    free(param);
    vTaskDelete(NULL);
}

// ------------------- HANDLER /padrao -------------------
static esp_err_t padrao_handler(httpd_req_t *req)
{
    char query[128];
    char padrao_str[32] = {0};
    char motor_1_str[2] = {0};
    char motor_2_str[2] = {0};
    char start_stop_str[2] = {0}; // Novo: para controlar Iniciar/Parar

    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing query string");
        return ESP_FAIL;
    }

    // Tenta obter o parâmetro 'stop'
    httpd_query_key_value(query, "stop", start_stop_str, sizeof(start_stop_str));
    
    // --- LÓGICA DE PARADA ---
    if (start_stop_str[0] == '1') { // Se o parâmetro 'stop=1' for recebido (clique em Parar)
        if (vibration_task_handle != NULL) {
            vTaskDelete(vibration_task_handle);
            vibration_task_handle = NULL;
            motors_off();
            ESP_LOGI(TAG, "Task de vibração interrompida por comando HTTP.");
        }
        const char *resp = "{\"status\":\"parado\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    // --- LÓGICA DE INÍCIO/REINÍCIO ---

    // Parar a task anterior se estiver rodando
    if (vibration_task_handle != NULL) {
        vTaskDelete(vibration_task_handle);
        vibration_task_handle = NULL;
        motors_off();
        ESP_LOGI(TAG, "Task anterior interrompida para iniciar novo padrão.");
    }
    
    // Tenta obter os parâmetros
    if (httpd_query_key_value(query, "seq", padrao_str, sizeof(padrao_str)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing 'seq' param");
        return ESP_FAIL;
    }
    httpd_query_key_value(query, "m1", motor_1_str, sizeof(motor_1_str));
    httpd_query_key_value(query, "m2", motor_2_str, sizeof(motor_2_str));


    // Prepara os parâmetros para a task (alocação dinâmica é necessária pois a task vai rodar por tempo indeterminado)
    blink_task_params_t *task_params = (blink_task_params_t *)malloc(sizeof(blink_task_params_t));
    if (task_params == NULL) {
        ESP_LOGE(TAG, "Falha ao alocar memória para os parâmetros da task.");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }

    strncpy(task_params->padrao, padrao_str, sizeof(task_params->padrao) - 1);
    task_params->motor_1_on = (motor_1_str[0] == '1');
    task_params->motor_2_on = (motor_2_str[0] == '1');

    // Cria a task com os novos parâmetros e armazena o handle globalmente
    xTaskCreate(blink_task, "blink_task", 4096, task_params, 5, &vibration_task_handle);

    ESP_LOGI(TAG, "Padrão iniciado. Handle: %p", vibration_task_handle);

    const char *resp = "{\"status\":\"executado\"}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// ------------------- PÁGINA HTML (JS Alterado para Iniciar/Parar e Repetição) -------------------
const char* INDEX_HTML = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESPWEBSERVER</title>
    <style>
        :root {
        /* 1. DECLARAÇÃO DA VARIÁVEL BASE */
        /* Definimos a unidade base em 5 pixels. */
        --tamanho-base: 10px; 
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: roboto sans-serif;
        }

        #container {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            width: 100vw;
            background-color: #f0f0f0;
        }

        #container-buttons {
            display: flex;
            flex-direction: column;
            flex-wrap: wrap;
            justify-content: center;
            align-items: center;
            gap: 10px;
            margin-bottom: 20px;
        }

        #container-wave {
            width: 80%;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
        }

        #buttons-wave {
            width: 80%;
            margin-top: 50px;
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            align-items: center;
            gap: 10px;
        }

        #buttons-control {
            width: 50%;
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 20px;
        }

        .btn, .btn-wave {
            padding: 10px 20px;
            border: none;
            border-radius: 5px;
            background-color: #007BFF;
            color: white;
            font-size: 16px;
            cursor: pointer;
            transition: background-color 0.3s ease;
        }

        .btn-wave:hover {
            background-color: red;
        }

        #wave {
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100px;
            width: 100%;
        }

        .onda-curta {
            width: var(--tamanho-base); 
            height: 60px;
            border-top: 3px solid #000; 
            border-left: 3px solid #000; 
            border-right: 3px solid #000; 
        }

        .onda-longa {
            width: calc(var(--tamanho-base) * 3); 
            height: 60px;
            border-top: 3px solid #000; 
            border-left: 3px solid #000; 
            border-right: 3px solid #000; 
        }
        .onda-gap {
            width: var(--tamanho-base); 
            height: 60px;
            border-bottom: 3px solid #000;
        }

        #LR-controll {
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 50px;
            margin-top: 15px;
            margin-bottom: 15px;
            padding: 10px;
            border: 1px solid #ccc;
            border-radius: 8px;
            background-color: white;
        }

        .slider {
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            gap: 10px;
        }

        .slider section {
             display: flex;
             width: 100%;
             justify-content: space-between;
             align-items: center;
             gap: 10px;
        }
    </style>
</head>
<body onload="load()">
    <section id="nav">

    </section>
    <section id="container">
        <div id="container-wave">
            <h2 id="waveText">Padrões</h2>
            <div id="wave">
                
            </div>
        </div>

        <div id="LR-controll">
            <div class="slider">
                <section style="display: flex; width: 100%; justify-content: space-between;">
                    <p>Motor Esquerdo (L)</p>
                    <input type="checkbox" name="radio_L" id="radio_L">
                </section>
            </div>

            <div class="slider">
                <section style="display: flex; width: 100%; justify-content: space-between;">
                    <p> Motor Direito (R)</p>
                    <input type="checkbox" name="radio_R" id="radio_R">
                </section>
            </div>
        </div>

        <div id="container-buttons">
            <div id="buttons-wave">
                <button class="btn-wave" data-pattern="00000">padrão 1</button>
                <button class="btn-wave" data-pattern="10000">padrão 2</button>
                <button class="btn-wave" data-pattern="11000">padrão 3</button>
                <button class="btn-wave" data-pattern="11100">padrão 4</button>
                <button class="btn-wave" data-pattern="11110">padrão 5</button>
                <button class="btn-wave" data-pattern="11111">padrão 6</button>
                <button class="btn-wave" data-pattern="01111">padrão 7</button>
                <button class="btn-wave" data-pattern="00111">padrão 8</button>
                <button class="btn-wave" data-pattern="00011">padrão 9</button>
                <button class="btn-wave" data-pattern="00001">padrão 10</button>
            </div>
            <div id="buttons-control">
                <button class="btn" onclick="startVibration(true)">Iniciar</button>
                <button class="btn" onclick="startVibration(false)">Parar</button>
            </div>
        </div>
    </section>
    <section id="footer">

    </section>
</body>
<script>

    var currentPadrao = "00000"; 
    var currentNamePadrao = "Padrão 1";

    function load(){
        console.log("carregou");
        document.getElementById("radio_L").checked = true;
        document.getElementById("radio_R").checked = true;
        
        document.querySelectorAll('.btn-wave').forEach((button, index) => {
            button.onclick = () => activateButton(button.getAttribute('data-pattern'), `Padrão ${index + 1}`);
        });

        activateButton("00000", "Padrão 1");
    }

    function startVibration(shouldStart) {
        
        // 1. Comando de Parada
        if (!shouldStart) {
            // Envia o comando 'stop=1' para o handler
            fetch(`/padrao?stop=1`)
                .then(response => response.json())
                .then(data => {
                    document.getElementById("waveText").innerHTML = "Parado";
                    console.log("Comando Parar enviado:", data);
                })
                .catch(err => {
                    console.error("Erro ao enviar comando Parar:", err);
                    document.getElementById("waveText").innerHTML = "Erro de comunicação ao Parar!";
                });
            return;
        }
        
        // 2. Comando de Início
        
        const motor1_on = document.getElementById("radio_L").checked ? '1' : '0';
        const motor2_on = document.getElementById("radio_R").checked ? '1' : '0';

        const padraoString = currentPadrao;
        
        // Constrói a URL de requisição com seq (padrão) e m1/m2 (estado dos motores)
        const url = `/padrao?seq=${padraoString}&m1=${motor1_on}&m2=${motor2_on}`;
        
        fetch(url)
            .then(response => response.json())
            .then(data => {
                document.getElementById("waveText").innerHTML = 
                    `${currentNamePadrao}`;
                console.log("Padrão iniciado:", data);
            })
            .catch(err => {
                console.error("Erro ao iniciar padrão:", err);
                document.getElementById("waveText").innerHTML = "Erro de comunicação ao Iniciar!";
            });
    }

    // Função para alterar o padrão (Chamada pelos botões de Padrão)
    function activateButton (pattern, name) {
        currentPadrao = pattern;
        currentNamePadrao = name;
        document.getElementById("waveText").innerHTML = name;
        updateWave();
    }


    function updateWave () {
        document.getElementById("wave").innerHTML = "";
        
        for (let index = 0; index < currentPadrao.length; index++) {
            // Cria o GAP (onda-gap)
            let div_gap = document.createElement("div");
            div_gap.className = "onda-gap";
            
            // Cria o PULSO
            let div_pulse = document.createElement("div");
            if(currentPadrao[index] == "1"){
                div_pulse.className = "onda-curta";
            } else if (currentPadrao[index] == "0") {
                div_pulse.className = "onda-longa";
            } else {
                div_pulse.className = "onda-gap";
            }
            
            document.getElementById("wave").appendChild(div_gap);
            document.getElementById("wave").appendChild(div_pulse);
        }

        // Adiciona um último GAP
        let div_gap_end = document.createElement("div");
        div_gap_end.className = "onda-gap";
        document.getElementById("wave").appendChild(div_gap_end);
    }
</script>
</html>
)rawliteral";

// --- ROOT HANDLER (Sem alteração) ---
static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static const httpd_uri_t root_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler,
    .user_ctx = NULL
};

static const httpd_uri_t padrao_uri = {
    .uri = "/padrao",
    .method = HTTP_GET,
    .handler = padrao_handler,
    .user_ctx = NULL
};

// ------------------- SERVIDOR, WIFI e MAIN (Sem alterações relevantes de lógica) -------------------
httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192;

    ESP_LOGI(TAG, "Iniciando servidor...");
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &root_uri);
        httpd_register_uri_handler(server, &padrao_uri);
        ESP_LOGI(TAG, "Handlers registrados.");
        return server;
    }
    ESP_LOGE(TAG, "Falha ao iniciar o servidor.");
    return NULL;
}

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Tentando reconectar...");
        } else {
            ESP_LOGI(TAG, "Falha na conexão Wi-Fi.");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "IP obtido: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        if (server == NULL) server = start_webserver();
    }
}

void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .pmf_cfg = {.capable = true, .required = false},
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Wi-Fi inicializado.");
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    led_init();
    motor_init();
    wifi_init_sta();
}