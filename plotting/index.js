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
const X_VALUES = [1, 2, 4, 8, 14, 16, 24, 28, 32, 48, 56];
const X_LABELS = [1, 2, 4, 8, 14, 28, 56];
/** @type {number[]} */ const X_LABELS_AL = Array.apply(null, { length: 17 }).map( (_, i) => Math.pow(2, i) );
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
    if (parallelCodePart >= 1.0) return numCores;
    const p = Math.min(parallelCodePart, 1.0), n = numCores;
    return 1 / ( (1-p) + (p/n) );
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
                format: path.extname(filepath).replace(".", ""),
                width: 400,
                height: 400
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
    for (let filename of LOG_FILENAMES) {
        if (filename.includes("unoptimized")) continue;

        console.log("\n------------------------------------------------------");
        console.log(`Processing logs for ${filename} ...`);
        const logFilepath = path.join(LOG_DIR, filename+".txt");
        filename = filename.replace("#", "s");
        const log = fs.readFileSync(logFilepath, "utf8");
        let scalingPlotname = "";
        let alPlotname = "";
        let scalingAxisRange;
        let filetypes = ["pdf"];

        // extract data
        /** @type {Map<number, number[]>} Cores => Times */
        const data = new Map();
        log.split("\n").map( l => l.split(";").map( w => w.trim() ) ).forEach( l => {
            if (scalingPlotname === "" && l[0].startsWith("title=")) scalingPlotname = l[0].replace("title=", "");
            if (alPlotname === "" && l[0].startsWith("title-AL=")) alPlotname = l[0].replace("title-AL=", "");
            if (l[0].startsWith("range=")) scalingAxisRange = JSON.parse(l[0].replace("range=", ""));
            if (l[0] === "svg") filetypes.push("svg");
            if (l.length !== 4 || isNaN(Number.parseInt(l[3]))) return;
            if (data.size === 0 && scalingPlotname === "") scalingPlotname = l[0];
            const cores = Number.parseInt(l[2]);
            if (!data.has(cores)) data.set(cores, []);
            data.get(cores).push(Number.parseInt(l[3]));
        });
        if (scalingPlotname === "") scalingPlotname = filename;

        // create filepath(s)
        const scalingPlotFilepaths = filetypes.map( f => path.join(PLOT_DIR, filename+"-scaling")+"."+f );
        const parallelCodePlotFilepaths = filetypes.map( f => path.join(PLOT_DIR, filename+"-amdahls-law")+"."+f );

        // check if valid logfile
        if (data.size === 0) {
            console.warn(`The log file "${filename}.txt" did not contain any valid lines! Skipping it`);
            continue;
        }

        // calculate mean time over all runs
        const meanData = new Map( Array.from(data).map( ([cores, times]) => [cores, times.reduce( (sum, t) => sum+t, 0 )/times.length] ) );
        console.log("Mean Times: " + Array.from(meanData).map( ([cores, time]) => `@${cores}=${time.toFixed()}μs` ).join(", "));
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

        const MARGIN = {
            l: 50,
            r: 0,
            b: 50,
            t: 50,
        };

        // create scaling plot
        for (const scalingPlotFilepath of scalingPlotFilepaths) if (!fs.existsSync(scalingPlotFilepath)) await createPlot(
            {
                data: [{
                    x,
                    y,
                    type: "scatter",
                    line: {
                        width: 3
                    },
                    marker: {
                        size: 6
                    }
                }],
                layout: {
                    xaxis: {
                        title: "<b>CPU-Cores</b>",
                        tickvals: X_LABELS,
                        tickmode: "array",
                        type: "log"
                    },
                    yaxis: {
                        title: "<b>Speedup</b>",
                        range: scalingAxisRange
                    },
                    title: `<b>${scalingPlotname}</b>`,
                    margin: MARGIN,
                }
            },
            scalingPlotFilepath
        );

        // create amdahl's law plot
        for (const parallelCodePlotFilepath of parallelCodePlotFilepaths) if (!fs.existsSync(parallelCodePlotFilepath)) await createPlot(
            {
                data: [
                    {
                        x: X_LABELS_AL,
                        y: X_LABELS_AL.map( n => amdahlsLaw(p28, n) ),
                        type: "scatter",
                        line: {
                            width: 1,
                            dash: "dot"
                        },
                        marker: {
                            size: 1
                        },
                        name: `Amdahl's Law; p=${(Math.min(p28, 1)*100).toFixed(2)}%`
                    },
                    p28 >= 1 && p56 >= 1 ? {} : {
                        x: X_LABELS_AL,
                        y: X_LABELS_AL.map( n => amdahlsLaw(p56, n) ),
                        type: "scatter",
                        line: {
                            width: 1,
                            dash: "dash"
                        },
                        marker: {
                            size: 1
                        },
                        name: `Amdahl's Law; p=${(Math.min(p56, 1)*100).toFixed(2)}%`
                    },
                    {
                        x,
                        y,
                        type: "scatter",
                        line: {
                            width: 1
                        },
                        marker: {
                            size: 4
                        },
                        name: "Measurement"
                    }
                ],
                layout: {
                    xaxis: {
                        title: "<b>CPU-Cores</b>",
                        tickvals: X_LABELS_AL,
                        tickmode: "array",
                        type: "log"
                    },
                    yaxis: {
                        title: "<b>Speedup</b>"
                    },
                    title: `<b>${alPlotname || "Amdahl's law"}</b>`,
                    showlegend: true,
                    margin: MARGIN,
                    legend: {
                        xanchor: "right",
                        x: 1,
                        y: 0.1
                    }
                }
            },
            parallelCodePlotFilepath
        );

        console.log("------------------------------------------------------");

    }

}
setImmediate(() => main().catch(console.error));
