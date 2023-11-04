const colorSelector = document.getElementById("color-selector");
const rainbowSpeedSlider = document.getElementById("rainbow-speed-slider");
const brightnessSlider = document.getElementById("brightness-slider");

window.addEventListener("load", setState, false);

function setState() {
    fetch("/state")
        .then(response => response.json())
        .then(function (data) {
            setSpeedState(data["speed"]);
            setColorState(data["color"]);
        })
        .catch(error => console.error(error));
}

function setSpeedState(speed) {
    rainbowSpeedSlider.value = speed;
}


const rgbToHex = (r, g, b) => "#" + ((1 << 24) | (r << 16) | (g << 8) | b).toString(16).slice(1).toUpperCase();

const hexToRgb = hex => {
    // Remove the '#' character from the hexadecimal color code if it exists.
    hex = hex.replace('#', '');

    // Split the hexadecimal color code into three parts.
    let r = hex.substring(0, 2);
    let g = hex.substring(2, 4);
    let b = hex.substring(4, 6);

    // Convert each part from hexadecimal to decimal.
    r = parseInt(r, 16);
    g = parseInt(g, 16);
    b = parseInt(b, 16);

    return {red: r, green: g, blue: b};
};

function setColorState(color) {
    colorSelector.value = rgbToHex(color["red"], color["green"], color["blue"]);
}

function setBrightnessState(brightness) {
    brightnessSlider.value = brightness;
}

function onColorClick() {
    fetch("/color", {
        method: "POST", headers: {"Content-Type": "application/json"},
        body: JSON.stringify({color: hexToRgb(colorSelector.value)})
    })
        .then(response => response.json())
        .then(data => setColorState(data["color"]))
        .catch(error => console.error(error));
}

function onRainbowClick() {
    fetch("/rainbow", {
        method: "POST", headers: {"Content-Type": "application/json"},
        body: JSON.stringify({rainbowSpeed: rainbowSpeedSlider.value})
    })
        .then(response => response.json())
        .then(data => setSpeedState(data["rainbowSpeed"]))
        .catch(error => console.error(error));
}

function onOffClick() {
    fetch("/off", {method: "POST"})
        .catch(error => console.error(error));

}

function onBrightnessClick() {
    fetch("/brightness", {
        method: "POST",
        headers: {"Content-Type": "application/json"},
        body: JSON.stringify({brightness: brightnessSlider.value})
    })
        .then(r => r.json())
        .then(data => setBrightnessState(data["brightness"]))
        .catch(error => console.error(error));
}

function onChristmasClick() {
    fetch("/christmas", {method: "POST"})
        .catch(error => console.error(error));
}