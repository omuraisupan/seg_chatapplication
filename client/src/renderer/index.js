const button = document.getElementById('connect')

button.addEventListener('click', () => {
    const ip = document.getElementById('ip').value;
    const port = document.getElementById('port').value;
    try {
        window.api.connectServer(ip, port);
        console.log("ip %s", ip);
        console.log("port %s", port);
        window.location.href = './chat.html';
    } catch(err) {
        console.error('error:'+ err);
    }
})