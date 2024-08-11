/*
	ο κώδικας υλοποιεί έναν server σε Node.js που δέχεται πίνακες (mazes) από μια συσκευή ESP32, 
	βρίσκει τη βέλτιστη διαδρομή από την αρχή μέχρι το τέλος του πίνακα με τον αλγόριθμο A*, 
	και διανέμει τα δεδομένα στους συνδεδεμένους χρήστες μέσω WebSocket. 
	Αν βρεθεί διαδρομή, επιστρέφεται στον client και διανέμεται μαζί με τον πίνακα στους συνδεδεμένους χρήστες
	μέσω WebSocket χρησιμοποιώντας το Socket.io
*/
// Λειτουργία A* για εύρεση της βέλτιστης διαδρομής στον πίνακα
function aStar(maze, start, end) {
	const rows = maze.length;
	const cols = maze[0].length;

	const directions = [
		[0, 1],
		[1, 0],
		[0, -1],
		[-1, 0],
	];

	function heuristic(a, b) {
		return Math.abs(a[0] - b[0]) + Math.abs(a[1] - b[1]);
	}

	const openSet = [
		{ pos: start, g: 0, f: heuristic(start, end), path: [start] },
	];
	const closedSet = new Set();

	while (openSet.length > 0) {
		openSet.sort((a, b) => a.f - b.f);
		const current = openSet.shift();

		if (current.pos[0] === end[0] && current.pos[1] === end[1]) {
			return current.path; // Επιστροφή της διαδρομής όταν φτάσει το τέρμα
		}

		closedSet.add(current.pos.toString());

		for (const [dx, dy] of directions) {
			const neighbor = [current.pos[0] + dx, current.pos[1] + dy];

			if (
				neighbor[0] < 0 ||
				neighbor[0] >= rows ||
				neighbor[1] < 0 ||
				neighbor[1] >= cols ||
				maze[neighbor[0]][neighbor[1]] === 1 ||
				closedSet.has(neighbor.toString())
			)
				continue;

			const g = current.g + 1;
			const f = g + heuristic(neighbor, end);
			const path = [...current.path, neighbor];

			openSet.push({ pos: neighbor, g, f, path });
		}
	}
	return null; // Επιστροφή null αν δεν βρεθεί διαδρομή
}

const express = require('express');
const bodyParser = require('body-parser');
const http = require('http');
const socketIo = require('socket.io');

const app = express();
const port = 3000;

// Δημιουργία HTTP server και ενσωμάτωση του Socket.io
const server = http.createServer(app);
const io = socketIo(server);

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

// Serve static files (e.g., HTML, CSS, JS)
app.use(express.static('public'));

// Αποθήκευση του τελευταίου πίνακα maze από το ESP32
let latestMaze = Array(20)
	.fill()
	.map(() => Array(20).fill(0)); // Προκαθορισμένος κενός πίνακας 10x10

// Επεξεργασία των POST αιτημάτων από το ESP32
app.post('/api/maze', (req, res) => {
	try {
		// Αποθήκευση του νέου πίνακα maze που λαμβάνεται από το αίτημα
		latestMaze = req.body.maze;

		console.log('Received maze:');
		console.table(latestMaze); // Εμφάνιση του πίνακα στον server

		const start = [0, 0];
		const end = [19, 19];
		const path = aStar(latestMaze, start, end);

		if (path) {
			res.json({ path });
		} else {
			res.status(404).send('No path found');
		}

		// Αποστολή του πίνακα maze σε όλους τους συνδεδεμένους clients
		io.emit('newMaze', latestMaze);

		//
		io.emit('newPath', path);

		res.status(200).send('Maze received successfully');
	} catch (error) {
		console.error('Error processing maze:', error);
		res.status(400).send('Invalid maze data');
	}
});

// Διαχείριση συνδέσεων WebSocket
io.on('connection', (socket) => {
	console.log('A client connected');

	// Αποστολή του τελευταίου πίνακα maze όταν ένας νέος client συνδεθεί
	if (latestMaze) {
		socket.emit('newMaze', latestMaze);
	}

	socket.on('disconnect', () => {
		console.log('Client disconnected');
	});
});

// Εκκίνηση του server
server.listen(port, () => {
	console.log(`Server is running on http://localhost:${port}`);
});
