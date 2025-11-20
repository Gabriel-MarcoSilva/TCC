var currentPadrao = "11111"; 
    var currentNamePadrao = "Padrão 5";

    function load(){
        console.log("carregou");
        document.getElementById("radio_L").checked = true;
        document.getElementById("radio_R").checked = true;
        
        document.querySelectorAll('.btn-wave').forEach((button, index) => {
            button.onclick = () => activateButton(button.getAttribute('data-pattern'), `Padrão ${index + 1}`);
        });

        activateButton("11111", "Padrão 5");
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