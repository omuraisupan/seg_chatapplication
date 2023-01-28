const msg_form = document.getElementById('msg_form');

msg_form.addEventListener('keydown', (e) => {
    if (e.keyCode === 13) {
        const msg = msg_form.value;
        window.api.sendMsg(msg);
        console.log('sendmsg:%s', msg);
        msg_form.value = '';
    }
})

window.api.receiveMsg((_event, msg) => {
    console.log('receive_msg: %s', msg);
    const newDiv = document.createElement('div');
    const newContent = document.createTextNode(msg);
    newDiv.appendChild(newContent);
    
    const currentDiv = document.getElementById('chatlog');
    currentDiv.after(newDiv);
})