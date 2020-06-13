// Modules to control application life and create native browser window
const {app, BrowserWindow, Menu, ipcMain} = require('electron')
const path = require('path')
const SerialPort = require('serialport')
const Readline = require('@serialport/parser-readline')
const port = new SerialPort('COM5', { baudRate: 115200 })
const parser = new Readline()
const Positioner = require('electron-positioner')
port.pipe(parser)

var temperature = "";
var humidity = "";
var heatIndex = "";
var ledColor = "";
var rainbowON = 0;

function createWindow () {
  // Create the browser window.
  const mainWindow = new BrowserWindow({
    width: 400,
    height: 165,
    webPreferences: {
      nodeIntegration: true
    },
  })

  // Set window's position.
  var positioner = new Positioner(mainWindow);
  positioner.move('bottomRight');
  mainWindow.setAlwaysOnTop(true);

  // and load the index.html of the app.
  mainWindow.loadFile('./resource/html/index.html');

  // Open the DevTools.
  // mainWindow.webContents.openDevTools()
  Menu.setApplicationMenu(null);
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.whenReady().then(() => {
  createWindow()
  
  app.on('activate', function () {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (BrowserWindow.getAllWindows().length === 0) createWindow()
  })
})

// Quit when all windows are closed, except on macOS. There, it's common
// for applications and their menu bar to stay active until the user quits
// explicitly with Cmd + Q.
app.on('window-all-closed', function () {
  if (process.platform !== 'darwin') app.quit()
})

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.
parser.on('data', function(line){
  if(line==null) return; 
  console.log(line);
  if(line[0]=='D'){
    data = line.split('D:')[1].split("-")
    temperature = data[0]
    humidity = data[1]
    heatIndex = data[2].split('\r')[0]
  }
})

ipcMain.on('getTemperature', (event, arg) => {
  event.returnValue = temperature;
})

ipcMain.on('getHumidity', (event, arg) => {
  event.returnValue = humidity;
})

ipcMain.on('getheatIndex', (event, arg) => {
  event.returnValue = heatIndex;
})

ipcMain.on('changeLEDColor', (event, arg) => {
  ledColor=arg;
  port.write("C:"+ledColor, function(err) {
    if (err) {
      return console.log('Error on write: ', err.message)
    }
  });
})

ipcMain.on('changeRainbow', (event, arg) => {
  rainbowON=arg;
  port.write("R:"+rainbowON, function(err) {
    if (err) {
      return console.log('Error on write: ', err.message)
    }
  });
})