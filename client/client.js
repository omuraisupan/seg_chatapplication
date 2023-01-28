const net = require('net');
const readline = require('readline');
const client = new net.Socket();

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

rl.question('IP address: ', (ip) => {
  rl.question('Port: ', (port) => {
    client.connect(port, ip, function() {
        console.log('Connected to: ' + ip + ':' + port);
        client.write('Hello, server! Love, Client.');
    });

    client.on('data', function(data) {
        console.log('Received: ' + data);
        client.destroy();
    });

    client.on('close', function() {
        console.log('Connection closed');
        rl.close();
    });
  });
});