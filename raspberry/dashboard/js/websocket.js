const socket = new WebSocket(
    `ws://${location.hostname}:3000`
);

socket.onopen=()=>{

    console.log("WebSocket Connected");

};

socket.onmessage=(event)=>{

    const data=JSON.parse(event.data);

    console.log(data);

};

socket.onclose=()=>{

    console.log("Disconnected");

};