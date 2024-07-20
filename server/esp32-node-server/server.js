const express = require('express');
const bodyParser = require('body-parser');
const http = require('http');
const socketIo = require('socket.io');

const app = express();
const port = 3000;

// Create an HTTP server and attach Socket.io
const server = http.createServer(app);
const io = socketIo(server);

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

// Serve static files (e.g., HTML, CSS, JS)
app.use(express.static('public'));

// Store the latest data from the ESP32
let latestData = { left_wall: null, right_wall: null, straight_wall: null };

// Handle POST requests from ESP32
app.post('/api/data', (req, res) => {
	latestData = {
		left_wall: req.body.left_wall,
		right_wall: req.body.right_wall,
		straight_wall: req.body.straight_wall,
	};

	console.log(`Received data - left_wall: ${latestData.left_wall},
                right_wall: ${latestData.right_wall},
                straight_wall: ${latestData.straight_wall}`);

	// Send the data to all connected clients
	io.emit('newData', latestData);

	res.status(200).send('Data received successfully');
});

// Handle WebSocket connections
io.on('connection', (socket) => {
	console.log('A client connected');

	// Send the latest data when a new client connects
	if (
		latestData.left_wall &&
		latestData.right_wall &&
		latestData.straight_wall
	) {
		socket.emit('newData', latestData);
	}

	socket.on('disconnect', () => {
		console.log('Client disconnected');
	});
});

// Start the server
server.listen(port, () => {
	console.log(`Server is running on http://localhost:${port}`);
});
