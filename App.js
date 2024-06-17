
import React, { useState, useEffect } from 'react';
import { Text, View, TextInput, Button } from 'react-native';
import TcpSocket from 'react-native-tcp-socket';

const App = () => {
  const [isConnected, setIsConnected] = useState(false);
  const [receivedMessage, setReceivedMessage] = useState('');
  const [messageToSend, setMessageToSend] = useState('');
  const [socket, setSocket] = useState(null);
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  


 
      const connectToESP32 = () => {
        const socket = TcpSocket.createConnection({
          port: port,
          host: 'ip',
          reuseAddress: true,
        });


      socket.on('connect', () => {
        setIsConnected(true);
        console.log("connection made");
      });

      socket.on('Appdata', (Appdata) => {
      setReceivedMessage(Appdata);
      console.log("msg" , Appdata);
       });

     let dataBuffer = [];

      socket.on('data', (data) => {
        const message = data.toString();
        dataBuffer.push(message);
        console.log(message);
        setReceivedMessage(message);
      });

      // Log the data buffer every 5 seconds 
      setInterval(() => {
        console.log(dataBuffer.join(''));
        dataBuffer = [];
      }, 50000);

      socket.on('error', (error) => {
        console.error('Socket error:', error);
      
      });

      socket.on('close', () => {
        // setIsConnected(false);
        console.log('Socket closed');
      });

      return socket;
    };

    useEffect(() => {
      const socket = connectToESP32();
      setSocket(socket);
    }, []);
  
    const handleSubmit = () => {
      const message = `username:${username},password:${password}`;
      socket.write(message);
      console.log("message sent: ", message);
    };


  const sendMessage = () => { 
    if (isConnected) {
      console.log("conAppdata" , messageToSend);
      socket.write(messageToSend);
      // setMessageToSend('');
    } else {
      console.warn('Not connected to ESP32');
    }
  };

  return (
    <View style={{ flex: 1, alignItems: 'center', justifyContent: 'center' }}>
      {isConnected ? (
        <Text>Connected</Text>
      )
       :
       
       (
        <Text>Connecting to ESP32...</Text>
      )}
       <TextInput
        placeholder="Username"
        value={username}
        onChangeText={(text) => setUsername(text)}
      />
      <TextInput
        placeholder="Password"
        secureTextEntry={true}
        value={password}
        onChangeText={(text) => setPassword(text)}
      />
      <Button title="Submit" onPress={handleSubmit} />

 
  
      <Text> {messageToSend}</Text>
      <TextInput
        placeholder="Enter message"
        value={messageToSend}
        onChangeText={handleChange}
      />
      <Button title="Send" onPress={sendMessage} />
    </View>
  );
};

export default App;



















































// import React, { useState, useEffect } from 'react';
// import { Text, View, TextInput, Button } from 'react-native';
// import TcpSocket from 'react-native-tcp-socket';
// import NetInfo from '@react-native-community/netinfo';
// import WifiRecon from 'react-native-wifi-recon';

// const App = () => {
//   const [isConnected, setIsConnected] = useState(false);
//   const [receivedMessage, setReceivedMessage] = useState('');
//   const [messageToSend, setMessageToSend] = useState('');
//   const [esp32Ip, setEsp32Ip] = useState('');
//   const [esp32Port, setEsp32Port] = useState(port);

//     const connectToESP32 = () => {
//       NetInfo.fetch().then(state => {
//         if (state.isConnected) {
//           WifiRecon.startScan().then(results => {
//             const esp32Device = results.find(
//               device => device.SSID === 'ESP32' || device.BSSID === 'ESP32_MAC_ADDRESS'
//             );
//             if (esp32Device) {
//               setEsp32Ip(esp32Device.IP);
//               setEsp32Port(port);
//             }
//           });
//         }
//       });
//       const socket = TcpSocket.createConnection({ host: esp32Ip, port: esp32Port });

//       socket.on('connect', () => {
//         setIsConnected(true);
//         console.log("connection made");
//       });

//       socket.on('Appdata', (Appdata) => {
//         setReceivedMessage(Appdata);
//         console.log("msg" , Appdata);
//       });

//       socket.on('error', (error) => {
//         console.error('Socket error:', error);
      
//       });

//       socket.on('close', () => {
//         // setIsConnected(false);
//         console.log('Socket closed');
//       });
//     };
//     useEffect(() => {
//     connectToESP32();
//   }, []);

//   const handleChange = (Appdata)=>{
//     setMessageToSend(Appdata)
//   }

//   const sendMessage = () => { 
//     if (isConnected) {
//       console.log("conAppdata" , messageToSend);
//       socket.write(messageToSend);
//       console.log("after conAppdata" , messageToSend);
//       // setMessageToSend('');
//     } else {
//       console.warn('Not connected to ESP32');
//     }
//   };

//   return (
//     <View style={{ flex: 1, alignItems: 'center', justifyContent: 'center' }}>
//       {isConnected ? (
//         <Text>Connected</Text>
//       )
//        :
       
//        (
//         <Text>Connecting to ESP32...</Text>
//       )}
//       <Text> {messageToSend}</Text>
//       <TextInput
//         placeholder="Enter message"
//         value={messageToSend}
//         onChangeText={handleChange}
//       />
//       <Button title="Send" onPress={sendMessage} />
//     </View>
//   );
// };

// export default App;















