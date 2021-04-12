"use strict";

const PLOTLY_CONFIG = require("./plotly-credentials.json");
const fs = require("fs");
const path = require("path");
const { pipeline } = require("stream");
const plotly = require("plotly")(PLOTLY_CONFIG.username, PLOTLY_CONFIG.apiKey);

// create dirs
const PLOT_DIR = path.join(__dirname, "plots");
const LOG_DIR = path.join(__dirname, "logs");
if (!fs.existsSync(PLOT_DIR)) fs.mkdirSync(PLOT_DIR);
if (!fs.existsSync(LOG_DIR)) fs.mkdirSync(LOG_DIR);

// array of log filenames without the ending ".csv"
const LOG_FILENAMES = fs.readdirSync(LOG_DIR, { withFileTypes: true }).filter( f => f.isFile() && f.name.endsWith(".csv") ).map( f => f.name.slice(0, -4) );

async function main() {

    // plot one image per log
    for (const filename of LOG_FILENAMES) {

        console.log(`Processing logs in ${filename}.csv ...`);
        const logFilepath = path.join(LOG_DIR, filename+".csv");
        const plotFilepath = path.join(PLOT_DIR, filename+".png");
        const log = fs.readFileSync(logFilepath, "utf8");
        let plotname = filename;

        // extract data
        /** @type {Map<number, number[]>} Cores => Times */
        const data = new Map();
        log.split("\n").map( l => l.split(";").map( w => w.trim() ) ).forEach( l => {
            if (l.length !== 4) return;
            if (data.size === 0) plotname = l[0];
            const cores = Number.parseInt(l[2]);
            if (!data.has(cores)) data.set(cores, []);
            data.get(cores).push(Number.parseInt(l[3]));
        });

        // check if valid logfile
        if (data.size === 0) {
            console.warn(`The log file "${filename}.csv" did not contain any valid lines! Skipping it`);
            continue;
        }

        // calculate mean time over all runs
        const meanData = new Map( Array.from(data).map( ([cores, times]) => [cores, times.reduce( (sum, t) => sum+t, 0 )/times.length] ) );

        // create data from log lines
        const basetime = meanData.get(1); // time im microsecs for 1 core
        /** @type {number[]} */ const x = [];
        /** @type {number[]} */ const y = [];
        for (const [cores, meanTime] of meanData) {
            x.push(cores);              // num cores
            y.push(basetime/meanTime);  // speedup
        }

        // create plot
        console.log("Creating plot ...");
        /** @type {import("http").IncomingMessage} */
        const imgStream = await new Promise( (resolve, reject) => 
            plotly.getImage(
                {
                    data: [{
                        x,
                        y,
                        type: "scatter",
                        line: {
                            width: 5
                        },
                        marker: {
                            size: 10
                        }
                    }],
                    layout: {
                        xaxis: {
                            title: "CPU-Cores"
                        },
                        yaxis: {
                            title: "Speedup"
                        },
                        title: plotname
                    }
                },
                {
                    format: "png",
                    width: 1000,
                    height: 500
                },
                (err, msg) => err ? reject(err) : resolve(msg)
            )
        );

        // write to fs
        const fileStream = fs.createWriteStream(plotFilepath);
        await new Promise( (resolve, reject) => pipeline(imgStream, fileStream, err => err ? reject(err) : resolve()) );
        console.log(`Created plot ${filename}.png`)

    }

}
setImmediate(main);