const { app, ipcMain, BrowserView, BrowserWindow } = require('electron');
const net = require('net');
const path = require('path');
const fs = require('fs');

let socket = new net.Socket();

const createWindow = () => {
  const mainWindow = new BrowserWindow({
    width: 1200,
    height: 800,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
    },
  });

  mainWindow.loadFile(path.join(__dirname, 'renderer/index.html'));

  mainWindow.on('close', () => {
    socket.write('exit');
    app.quit();
  })

  // Opne Devtools
  //mainWindow.webContents.openDevTools(/*{ mode: 'detach'}*/);
}

app.whenReady().then(() => {
  createWindow();

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  })
})

ipcMain.on('connet_server', (event, ip, port) => {
    socket.connect(port, ip, () => {
      console.log("connect to: %s %s", ip, port);
    })

    socket.on('data', (data) => {
      console.log('receive_msg-> ' + data.toString());
      event.sender.send('receive_msg', data.toString());

      if (data.toString().startsWith("!p_")) {
        const pagesJson = fs.readFileSync(path.join(__dirname, 'pages.json'));
        const in_word = data.toString().substring(3);
        if (in_word in JSON.parse(pagesJson)) {
          let win = new BrowserWindow({width: 800, height: 600})
          win.on('closed', () => {
            win = null
          });
          const view = new BrowserView();
          win.setBrowserView(view);
          view.setBounds({ x: 0, y: 0, width: 800, height: 600 });
          view.webContents.loadURL(JSON.parse(pagesJson)[in_word]);
          view.setAutoResize({width: true, height: true});
        };
      };
    });
    
    socket.on('end', () => {
      socket.end();
    });

    socket.on('error', (err) => {
      console.error(err);
    });
})

ipcMain.on('send_msg', (_event, msg) => {
  console.log("send_msg:" + msg.toString());
  socket.write(msg);
})
