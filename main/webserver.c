#include "webserver.h"
#include "vibration.h"
#include <esp_log.h>
#include <string.h>
#include <esp_http_server.h>

static const char *TAG = "WEBSERVER";

/* INDEX_HTML: aqui está o HTML/CSS/JS completo que você enviou */
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

/* Handlers */
static esp_err_t root_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t padrao_handler(httpd_req_t *req)
{
    char query[128];
    char padrao_str[32] = {0};
    char motor_1_str[2] = {0};
    char motor_2_str[2] = {0};
    char start_stop_str[2] = {0};

    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing query string");
        return ESP_FAIL;
    }

    httpd_query_key_value(query, "stop", start_stop_str, sizeof(start_stop_str));
    if (start_stop_str[0] == '1') {
        vibration_stop();
        const char *resp = "{\"status\":\"parado\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    if (httpd_query_key_value(query, "seq", padrao_str, sizeof(padrao_str)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing 'seq' param");
        return ESP_FAIL;
    }
    httpd_query_key_value(query, "m1", motor_1_str, sizeof(motor_1_str));
    httpd_query_key_value(query, "m2", motor_2_str, sizeof(motor_2_str));

    vibration_start(padrao_str, motor_1_str[0]=='1', motor_2_str[0]=='1');

    const char *resp = "{\"status\":\"executado\"}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
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

httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192;
    static httpd_handle_t server = NULL;

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