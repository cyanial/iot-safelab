import axios from 'axios'


// Make a request for a user with a given ID
// axios.get('/api/v1')

// id = setInterval(() => {console.log('a')}, 1000)

export function getDevMesg() {
    axios.get("/api/v1/devmesg")
        .then(function(response) {
            // handle success
            console.log(response);
        })
        .catch(function (error) {
            // handle error
            console.log(error);
        })
        .then(function() {
            // always executed
        })
}
