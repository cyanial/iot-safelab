import * as React from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';
import Title from './Title';

import axios from 'axios'


export default class Chart extends React.Component {

    // setInterval(() => getDevMesg(), 3000);

    constructor(props) {
        super(props);

        this.data_list = [];
        this.state = {
            data: [],
        };

        setInterval(this.getDevMesg.bind(this), 3000);
        setInterval(this.updateState.bind(this), 3000);

    }

    updateState() {
        this.setState({
            data: this.data_list.slice(-30)
        })
        this.forceUpdate()
    }

    getDevMesg() {

        var self = this;
        axios.get("/api/v1/devmesg")
            .then(function (response) {
                // handle success
                // console.log(response);
                // console.log(response.data.shadow);
                let sd = response.data.shadow;
                let report = (sd[0]).reported;
                let data_pack = report['properties'];
                // console.log(report);
                // console.log(data_pack);
                self.data_list.push({
                            time: report['event_time'],
                            smoke_value: data_pack['Smoke_Value'],
                            temperature: data_pack['Temperature'],
                            humidity: data_pack['Humidity'],
                            luminance: data_pack['Luminance'],
                            beep: data_pack['BeepStatus'],
                            door: data_pack['DoorStatus'],
                        });      
                
                console.log(self.data_list);
            })
            .catch(function (error) {
                // handle error
                console.log(error);
            })
            .then(function () {
                // always executed
            });

            
    }


    render() {
        return (
            <React.Fragment>
                <Title>Today</Title>
                <ResponsiveContainer width="100%" height="100%">
                    <LineChart
                        width={500}
                        height={300}
                        data={this.state.data}
                        margin={{
                            top: 5,
                            right: 30,
                            left: 20,
                            bottom: 5,
                        }}
                    >
                        <CartesianGrid strokeDasharray="3 3" />
                        <XAxis dataKey="time" />
                        <YAxis />
                        <Tooltip />
                        <Legend />
                        <Line type="monotone" dataKey="smoke_value" stroke="#8884d8" activeDot={{ r: 8 }} />
                        <Line type="monotone" dataKey="temperature" stroke="#82ca9d" />
                        <Line tppe="monotone" dataKey="humidity" stroke="#12ca9d" />
                        <Line tppe="monotone" dataKey="luminance" stroke="#42c29d" />
                    </LineChart>
                </ResponsiveContainer>
            </React.Fragment>
        );
    }
}