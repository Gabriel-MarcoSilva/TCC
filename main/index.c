const char* INDEX_HTML = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESPWEBSERVER</title>
</head>
<body onload="load()">
    <section id="nav">

    </section>
    <section id="container">
        <div id="wave">
        </div>

        <div id="LR-controll">
            <div class="slider">
                <p>L</p>
                <input type="range" min="0" max="255" value="127" class="slider" id="sliderL">
                <input type="checkbox" name="radio_L" id="radio_L">
            </div>

            <div class="slider">
                <p>R</p>
                <input type="range" min="0" max="255" value="127" class="slider" id="sliderR">
                <input type="checkbox" name="radio_R" id="radio_R">
            </div>
        </div>

        <div id="container-buttons">
            <button class="btn-wave">padrão 1</button>
            <button class="btn-wave">padrão 2</button>
            <button class="btn-wave">padrão 3</button>
            <button class="btn-wave">padrão 4</button>
            <button class="btn-wave">padrão 5</button>
            <button class="btn-wave">padrão 6</button>
            <button class="btn-wave">padrão 7</button>
            <button class="btn-wave">padrão 8</button>
        </div>
    </section>
    <section id="footer">

    </section>
</body>
<script>
    function load(){
        alert("olá Gabriel")
    }
</script>
</html>
)rawliteral";