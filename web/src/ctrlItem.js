import * as React from 'react';
import Title from './Title';
import { Button } from '@mui/material';
import axios from 'axios';



export class BeepCtrl extends React.Component {

    constructor(props) {
        super(props);
    }


    handleBeepON(e) {
        e.preventDefault();
        axios.post('/api/v1/beepon', {})
            .then(function (response) {
                console.log(response);
            })
            .catch(function (error) {
                console.log(error);
            });
    }

    handleBeepOFF(e) {
        e.preventDefault();
        axios.post('/api/v1/beepoff', {})
            .then(function (response) {
                console.log(response);
            })
            .catch(function (error) {
                console.log(error);
            });
    }

    render() {
        return (
            <React.Fragment>
                <Title>Beep Control</Title>
                <Button color="primary" onClick={this.handleBeepON}>
                    ON
                </Button>
                <Button color="secondary" onClick={this.handleBeepOFF}>
                    OFF
                </Button>
            </React.Fragment>
        );
    }
}

export class DoorCtrl extends React.Component {
    constructor(props) {
        super(props);
    }

    handleDoorON(e) {
        e.preventDefault();
        axios.post('/api/v1/dooron', {})
            .then(function (response) {
                console.log(response);
            })
            .catch(function (error) {
                console.log(error);
            });
    }

    handleDoorOFF(e) {
        e.preventDefault();
        axios.post('/api/v1/dooroff', {})
            .then(function (response) {
                console.log(response);
            })
            .catch(function (error) {
                console.log(error);
            });
    }

    render() {
        return (
            <React.Fragment>
                <Title>Door Control</Title>
                <Button color="primary" onClick={this.handleDoorON}>
                    ON
                </Button>
                <Button color="secondary" onClick={this.handleDoorOFF}>
                    OFF
                </Button>
            </React.Fragment>
        );
    }
}

