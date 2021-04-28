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

// array of log filenames without the ending ".txt"
const LOG_FILENAMES = fs.readdirSync(LOG_DIR, { withFileTypes: true }).filter( f => f.isFile() && f.name.endsWith(".txt") ).map( f => f.name.slice(0, -4) );

/** @type {number[]} Contains the numbers 2^x for x = 0..16 */
const AMDAHLS_LAW_X = [1, 2, 4, 8, 14, 16, 24, 28, 32, 48, 56];
//for (let i=0;i<=16;i++) AMDAHLS_LAW_X.push(Math.pow(2, i));

/**
 * Calculates the parallelized code part of the measured program
 * after Amdahl's Law
 * @param {number} speedup The speedup in comparison to using
 * only one CPU core
 * @param {number} numCores The number of cores the was used
 * when the given speedup was achieved
 */
function getParallelCodeBySpeedup( speedup, numCores ) {
    const s = speedup, n = numCores;
    return (n/s) * (1/(1-n)) - (n/(1-n));
}

/**
 * Calculates the speedup with the given parallel code base
 * portion and the number of cores
 * @param {number} parallelCodePart 
 * @param {number} numCores The number of cores the was used
 * when the given speedup was achieved
 */
function amdahlsLaw( parallelCodePart, numCores ) {
    const p = Math.min(parallelCodePart, 1.0), n = numCores;
    return 1 / ( 1.0 + (p/n) );
}

/**
 * @param {{ data: import("plotly.js").Data[], layout: import("plotly.js").Layout }} figure 
 * @param {import("fs").PathLike} filepath 
 */
async function createPlot( figure, filepath ) {

    /** @type {import("http").IncomingMessage} */
    const imgStream = await new Promise( (resolve, reject) => 
        plotly.getImage(
            figure,
            {
                format: "png",
                width: 1000,
                height: 500
            },
            (err, msg) => err ? reject([err, msg]) : resolve(msg)
        )
    );

    // write to fs
    const fileStream = fs.createWriteStream(filepath);
    await new Promise( (resolve, reject) => pipeline(imgStream, fileStream, err => err ? reject(err) : resolve()) );
    console.log(`Created plot ${path.basename(filepath)}`);

}

async function main() {

    // plot one image per log
    for (const filename of LOG_FILENAMES) {

        console.log(`Processing logs for ${filename} ...`);
        const logFilepath = path.join(LOG_DIR, filename+".txt");
        const scalingPlotFilepath = path.join(PLOT_DIR, filename+"-scaling.png");
        const parallelCodePlotFilepath = path.join(PLOT_DIR, filename+"-amdahls-law.png");
        const log = fs.readFileSync(logFilepath, "utf8");
        let scalingPlotname = filename;

        // extract data
        /** @type {Map<number, number[]>} Cores => Times */
        const data = new Map();
        log.split("\n").map( l => l.split(";").map( w => w.trim() ) ).forEach( l => {
            if (l.length !== 4 || isNaN(Number.parseInt(l[3]))) return;
            if (data.size === 0) scalingPlotname = l[0];
            const cores = Number.parseInt(l[2]);
            if (!data.has(cores)) data.set(cores, []);
            data.get(cores).push(Number.parseInt(l[3]));
        });

        // check if valid logfile
        if (data.size === 0) {
            console.warn(`The log file "${filename}.txt" did not contain any valid lines! Skipping it`);
            continue;
        }

        // calculate mean time over all runs
        const meanData = new Map( Array.from(data).map( ([cores, times]) => [cores, times.reduce( (sum, t) => sum+t, 0 )/times.length] ) );
        const basetime = meanData.get(1); // time im microsecs for 1 core

        // calculate amdahls law related data
        const amdahlsLawData = new Map( Array.from(meanData).map( ([cores, time]) => [cores, getParallelCodeBySpeedup(basetime/time, cores)] ) );
        const p28 = amdahlsLawData.get(28); const p56 = amdahlsLawData.get(56);
        console.log(`Parellized code is ${(p28*100).toFixed(2)}% @28 cores and ${(p56*100).toFixed(2)}% @56 cores`);

        // create data from log lines
        /** @type {number[]} */ const x = [];
        /** @type {number[]} */ const y = [];
        for (const [cores, meanTime] of meanData) {
            x.push(cores);              // num cores
            y.push(basetime/meanTime);  // speedup
        }

        // create scaling plot
        if (!fs.existsSync(scalingPlotFilepath)) await createPlot(
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
                        title: "CPU-Cores",
                        tickvals: x,
                        tickmode: "array",
                        type: "log"
                    },
                    yaxis: {
                        title: "Speedup"
                    },
                    title: scalingPlotname
                }
            },
            scalingPlotFilepath
        );

        // create amdahl's law plot
        if (!fs.existsSync(parallelCodePlotFilepath)) await createPlot(
            {
                data: [
                    {
                        x: AMDAHLS_LAW_X,
                        y: AMDAHLS_LAW_X.map( n => amdahlsLaw(p28, n) ),
                        type: "scatter",
                        line: {
                            width: 2
                        },
                        marker: {
                            size: 0
                        },
                        name: `Amdahl's Law; p=${(p28,100).toFixed(2)}%`
                    },
                    {
                        x: AMDAHLS_LAW_X,
                        y: AMDAHLS_LAW_X.map( n => amdahlsLaw(p56, n) ),
                        type: "scatter",
                        line: {
                            width: 2,
                            dash: "dash"
                        },
                        marker: {
                            size: 0
                        },
                        name: `Amdahl's Law; p=${(p56, 100).toFixed(2)}%`
                    },
                    {
                        x,
                        y,
                        type: "scatter",
                        line: {
                            width: 2,
                            dash: "dot"
                        },
                        marker: {
                            size: 6
                        },
                        name: "Measurement"
                    }
                ],
                layout: {
                    xaxis: {
                        title: "CPU-Cores",
                        tickvals: AMDAHLS_LAW_X,
                        tickmode: "array",
                        type: "log"
                    },
                    yaxis: {
                        title: "Speedup"
                    },
                    title: "Amdahl's law",
                    showlegend: true
                }
            },
            parallelCodePlotFilepath
        );

    }

}
setImmediate(main);
