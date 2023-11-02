const colorSelector = document.getElementById("color-selector");
const rainbowSpeedSlider = document.getElementById("rainbow-speed-slider");

// window.addEventListener("load", setColorPickerValue, false);
window.addEventListener("load", setState, false);

function setState() {
    // TODO get current state from backend
    // TODO set current color value in color picker
    // TODO set current rainbow speed in rainbow speed slider
    // TODO set values of slider and color picker in UI
    // TODO set button availability depending on the current effect
    colorSelector.value = "#008080";
    rainbowSpeedSlider.value = 66;
}

function onColorClick() {
    // TODO send POST request to /color with color information
    console.log("Color clicked");
    const color = colorSelector.value;
    console.log(color);
}

function onRainbowClick() {
    // TODO send POST request to /rainbow with rainbow speed information
    console.log("Rainbow clicked");
}

function onOffClick() {
    // TODO send POST request to /off
    console.log("Off clicked");
}