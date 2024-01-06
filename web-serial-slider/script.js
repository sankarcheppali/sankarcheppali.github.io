let port = null;
let outputDone = null;
let outputStream = null;

async function getPort() {
    return await navigator.serial.requestPort();
}

async function openSerialPort(){
  console.log("requesting serial port");
  port = await this.getPort()
  await port.open({ baudRate: 115200 });
  const encoder = new TextEncoderStream();
  outputDone = encoder.readable.pipeTo(port.writable);
  outputStream = encoder.writable;
  console.log("serial port opened");
}

async function closeSerialPort() {
  if (outputStream) {
    await outputStream.getWriter().close();
    await outputDone;
    outputStream = null;
    outputDone = null;
  }
  if(port!=null){
   await port.close();
    port = null; 
  }
  console.log("serial port closed");
}


function sendAngle(angle) {
  console.log("setting servo to "+angle);
  if (port == null) {
    console.log("port is not ready");
    return
  }
  let cmd = `:${angle}\n`;
  const writer = outputStream.getWriter();
  writer.write(cmd);
  writer.releaseLock();
}


$("#handle2").roundSlider({
    sliderType: "min-range",
    radius: 130,
    showTooltip: false,
    width: 16,
    value: 0,
    handleSize: 0,
    handleShape: "square",
    circleShape: "half-top",
    valueChange: function (args) {
        let angle = Math.ceil(args.value * 1.8)
        sendAngle(angle);
    }
});