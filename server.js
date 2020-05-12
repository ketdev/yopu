const express = require('express');
const path = require('path');
const app = express()
const router = express.Router();
const port = 3000

app.use(express.static('dist'));

router.get('/', function (req, res) {
    res.sendFile(path.join(__dirname + '/dist'));
});

app.listen(port, () => console.log(`Listening at http://localhost:${port}`));
