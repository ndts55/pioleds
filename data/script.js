const colorSelector = document.getElementById("color-selector");
const rainbowSpeedSlider = document.getElementById("rainbow-speed-slider");
const brightnessSlider = document.getElementById("brightness-slider");

window.addEventListener("load", setState, false);

function setState() {
    fetch("/state")
        .then(r => r.json())
        .then(data => setStateData(data))
        .catch(error => console.error(error));
}

function setStateData(data) {
    rainbowSpeedSlider.value = data["speed"];
    colorSelector.value = rgbToHex(data["color"]["red"], data["color"]["green"], data["color"]["blue"]);
    brightnessSlider.value = data["brightness"];
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

function onColorClick() {
    fetch("/color", {
        method: "POST", headers: {"Content-Type": "application/json"},
        body: JSON.stringify({color: hexToRgb(colorSelector.value)})
    })
        .then(r => r.json())
        .then(data => setStateData(data))
        .catch(error => console.error(error));
}

function onRainbowClick() {
    fetch("/rainbow", {
        method: "POST", headers: {"Content-Type": "application/json"},
        body: JSON.stringify({rainbowSpeed: rainbowSpeedSlider.value})
    })
        .then(r => r.json())
        .then(data => setStateData(data))
        .catch(error => console.error(error));
}

function onOffClick() {
    fetch("/off", {method: "POST"})
        .then(r => r.json())
        .then(data => setStateData(data))
        .catch(error => console.error(error));

}

function onBrightnessClick() {
    fetch("/brightness", {
        method: "POST",
        headers: {"Content-Type": "application/json"},
        body: JSON.stringify({brightness: brightnessSlider.value})
    })
        .then(r => r.json())
        .then(data => setStateData(data))
        .catch(error => console.error(error));
}

function onChristmasClick() {
    fetch("/christmas", {method: "POST"})
        .then(r => r.json())
        .then(data => setStateData(data))
        .catch(error => console.error(error));
}