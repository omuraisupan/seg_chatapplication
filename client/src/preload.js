const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
  sendMsg: (msg) => ipcRenderer.send('send_msg', msg),
  receiveMsg: (msg) => ipcRenderer.on('receive_msg', msg),
  connectServer: (ip, port) => ipcRenderer.send('connet_server', ip, port),
});