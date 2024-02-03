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

function sendTokenNumber(tokenNumber) {
  console.log("setting token number "+tokenNumber);
  if (port == null) {
    console.log("port is not ready");
    return
  }
  let cmd = `:${tokenNumber}\n`;
  const writer = outputStream.getWriter();
  writer.write(cmd);
  writer.releaseLock();
}

function readTokenNumber(){
  let elem = document.getElementById("token_number");
  if(elem.value){
    return parseInt(elem.value);
  }
  return 0;
}

function setTokenNumber(tokenNumber) {
  let elem = document.getElementById("token_number");
  elem.value = `${tokenNumber}`;
}

function incrementTokenNumber(){
  let tokenNumber = readTokenNumber()+1
  setTokenNumber(tokenNumber)
  sendTokenNumber(tokenNumber)
}

function decrementTokenNumber(){
  let tokenNumber = readTokenNumber()-1
  setTokenNumber(tokenNumber)
  sendTokenNumber(tokenNumber)
}

function updateTokenNumber(){
 sendTokenNumber(readTokenNumber())
}


