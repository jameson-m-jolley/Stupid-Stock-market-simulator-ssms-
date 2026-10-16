// Adjectives & nouns for stock names
const adjectives = [
  "Global", "National", "International", "Dynamic", "Quantum", "Prime", "Apex", "Summit",
  "NextGen", "Future", "United", "Central", "Pioneer", "Superior", "Frontier", "Nova",
  "Titan", "Infinite", "Blue", "Green", "Crimson", "Golden", "Silver", "Iron", "Rapid",
  "Swift", "Visionary", "Digital", "Modern", "Advanced", "Elite", "Core", "Strategic",
  "Vertex", "Atlas", "Solar", "Lunar", "Eco", "Primeval", "Omni", "Metro", "Urban",
  "Rural", "Continental", "Universal", "Aurora", "Zenith", "Polar", "Alpha", "Omega",
  "Sun"
];

const nouns = [
  "Technologies", "Industries", "Systems", "Holdings", "Enterprises", "Dynamics", "Corporation",
  "Labs", "Solutions", "Networks", "Resources", "Logistics", "Foods", "Energy", "Pharma",
  "Healthcare", "Biotech", "Mining", "Aerospace", "Automotive", "Software", "Hardware",
  "Finance", "Capital", "Securities", "Investments", "Properties", "Realty", "Infrastructure",
  "Construction", "Manufacturing", "Metals", "Plastics", "Retail", "Textiles", "Agriculture",
  "Farms", "Media", "Entertainment", "Gaming", "Communications", "Electronics", "Shipping",
  "Transportation", "Waterworks", "Petroleum", "Chemicals", "Research", "Consulting",
  "Analytics", "Automation", "Defense", "Robotics", "AI"
];

// Class definitions
class Candle {
    constructor(date, open, high, low, close, trades) {
        this.d = date;
        this.o = open;
        this.h = high;
        this.l = low;
        this.c = close;
        this.t = trades;
    }
}

class Stock {
    constructor() {
        this.volume = 0;
        this.latestPrice = null;
    }

    setLatestPrice(price) {
        this.latestPrice = price;
    }

    getLatestPrice() {
        return this.latestPrice;
    }
}

// Model
class SimulationModel {
    constructor() {
        this.host = "localhost"
        this.ws = null;
        this.connected = false;

        // debug counts for performance
        this.msgcount = 0;
        this.candlecount = 0;
        this.totaltradescount = 0;

        // State
        this.stocks = [];
        this.num_stocks = 0;
        this.isFirstConnect = true;
        this.simulationStartTime = null;

        // Playback functionality
        this.playbackMode = false;
        this.playbackData = null;
        this.playbackIndex = 0;
        this.playbackTimer = null;

        // Callbacks for controller
        this.onStartReceived = null;
        this.onUpdateReceived = null;

        this.onConnectionChange = null;
        this.onSimulationStateChange = null;
        this.onUserStateChange = null;
        this.onPlayBackSpeedChange = null;

        this.onLog = null;
    }

    connect() {
        if (this.ws) {
            this.ws.onopen = null;
            this.ws.onclose = null;
            this.ws.onerror = null;
            this.ws.onmessage = null;
            if (this.ws.readyState === WebSocket.OPEN || this.ws.readyState === WebSocket.CONNECTING) {
                this.ws.close();
            }
        }

        this.ws = new WebSocket(`ws://${this.host}:8080`);
        
        this.ws.onopen = () => {
            this.connected = true;
            this.isFirstConnect = false;
            this._notifyConnectionChange(true);
            this._log("Connected to server");

            //loading screen effect
            const loader = document.getElementById('loader-screen');
            const content = document.getElementById('main-content');

            loader.classList.add('hidden');
            setTimeout(() => loader.style.display = 'none', 1500);

            content.style.display = 'block';
        };
        
        this.ws.onmessage = (e) => {
            this._handleMessage(e.data);
        };
        
        this.ws.onclose = (e) => {
            this.connected = false;
            this._notifyConnectionChange(false);
            this._log(`Disconnected: code: ${e.code}`);
        };
        
        this.ws.onerror = (e) => {
            console.error("WebSocket error:", e);
        };
    }

    reconnect() {
        if (!this.connected) {
            this._log("Attempting to reconnect...");
            this.connect();
        }
    }

    disconnect() {
        if (this.ws) {
            this.ws.close();
        }
    }

    send_message(msg) {
        if (this.playbackMode) {
            this._log(`Cannot send commands during playback`);
            return false;
        }
        if (this.connected && this.ws) {
            this.ws.send(msg);
            return true;
        } else {
            this._log(`Error: not connected`);
            return false;
        }
    }

    getStock(index) {
        return this.stocks[index] || null;
    }
    
    getAllStocks() {
        return this.stocks;
    }
    
    isConnected() {
        return this.connected || this.playbackMode;
    }
    
    getNumStocks() {
        return this.num_stocks;
    }
    
    initialize_stocks(numStocks) {
        this.num_stocks = numStocks;
        this.stocks = [];
        for (let i = 0; i < numStocks; i++) {
            this.stocks.push(new Stock());
        }
    }
    
    clear_stocks() {
        this.stocks = [];
        this.num_stocks = 0;
        this.simulationStartTime = null; 
    }
    
    update_stock(index, candlesData) {
        if (!this.simulationStartTime) {
            console.error('simulationStartTime is NULL! Initializing to Date.now()');
            this.simulationStartTime = Date.now();
        }

        if (index >= 0 && index < this.stocks.length) {
            const sortedCandles = [...candlesData].sort((a, b) => a.close_time - b.close_time);

            for (let candleData of sortedCandles) {
                const { open, high, low, close, num_trades, close_time } = candleData;

                if (!Number.isFinite(close_time) || close_time <= 0 || num_trades === 0) continue;

                const timestamp = new Date(Number(this.simulationStartTime) + close_time / 1_000_000);

                const t = timestamp.getTime();
                if (Number.isNaN(t) || t <= 0) {
                    console.warn(`Skipping candle due to invalid timestamp: ${timestamp}`);
                    continue;
                }

                const candle = new Candle(timestamp, open, high, low, close, num_trades);

                if (!this.stocks[index]) continue;

                if (this.onStockUpdate) {
                    this.onStockUpdate(index, candle);
                }
            }
        }
    }

    requestCandles(stockIndex, amountOfCandles = null) {
        if (!this.connected || this.playbackMode) return;
        
        const json = this.makeCommand("get")("candles")({
            stock: stockIndex,
            amountOfCandles: amountOfCandles
        });

        this.send_message(JSON.stringify(json));
    }

    // Socket message handling
    _handleMessage(data) {
        try {
            const message = JSON.parse(data);

            if (message.local) {
                if (message.local === "stats") {
                    this._debugLog();
                }
                return;
            }

            this.msgcount++;

            // User messages
            if (message.user) {
                this._handleUserMessage(message.user);
                return;
            }

            // Server state messages
            if (message.server) {
                this._handleServerCommand(message.server);
                return;
            }

            // Current simulation messages
            if (message.sim) {
                if (message.sim.start) {
                    const info = message.sim.start;
                    this.simulationStartTime = Date.now();
                    if (this.onStartReceived) {
                        this.onStartReceived(info.num_stocks, info.init_wallet, info.time_passed);
                    }
                    return;
                }

                if (message.sim.update) {
                    const info = message.sim.update;
                    if (this.onUpdateReceived) {
                        this.onUpdateReceived(info.status, info.time_passed, info.stock_data);
                    }

                    this.totaltradescount = info.stock_data.volumes
                        ? info.stock_data.volumes.reduce((a, b) => a + b, 0)
                        : 0;
                    
                    return;
                }

                if (message.sim.candles) {
                    const info = message.sim.candles;
                    this.update_stock(info.stock, info.candles);
                    this.candlecount += this.candlecount = info.candles.length; 
                    return;
                }

                this._log(`Unrecognized sim command`);
            }

            // Get messages
            if (message.get) {
                this._log(`Get received: ${message.get.target}`);
                console.log(message.get.target, message.get.value);
                return;
            }
        } catch (err) {
            this._log("Error processing message: " + err.message);
            console.error(err);
        }
    }

    _debugLog() {
        console.group(`Stats @ ${new Date().toLocaleString()}`);
        console.log("Trades:", this.totaltradescount);
        console.log("Candles:", this.candlecount);
        console.log("Messages:", this.msgcount);
        console.groupEnd();
    }

    _handleUserMessage(update) {
        const tradeStatus = update.tradeStatus;
        switch (tradeStatus) {
            case "queued":
                this._log(`Trade queued`);
                break;
            case "on_market":
                this._log(`Trade on market`);
                break;
            case "filled":
                this._log(`Trade filled`);
                this.onUserStateChange(update.trade);
                break;
            case "error":
                this._log(`error trading: ${update.error}`);
                break;
            default:
                this._log("Unknown user msg");
        }
    }
    
    _handleServerCommand(command) {
        const boolValue = command.value === "1" ? true : false;
        const stateChanges = {
            "running": { type: "started", value: boolValue},
            "paused": { type: "paused", value: boolValue},
            "should_terminate": { type: "terminated", value: boolValue}
        }
        
        const change = stateChanges[command.target];
        if (change && this.onSimulationStateChange) {
            this.onSimulationStateChange(change.type, change.value);
            this._log(`Server ${command.target}: ${command.value}`);
        } else {
            this._log(`Server ${command.target}: ${command.value}`);
        }
    }
    
    _notifyConnectionChange(connected) {
        if (this.onConnectionChange) {
            this.onConnectionChange(connected);
        }
    }
    
    _log(message) {
        if (this.onLog) {
            this.onLog(message);
        }
    }

    _saveSimulation() {
        const simulationData = {
            num_stocks: this.num_stocks,
            stockData: this.stocks
        }

        const json = JSON.stringify(simulationData, null, 2);
        const blob = new Blob([json], {type: `application/json`});

        const url = URL.createObjectURL(blob);
        const a = document.createElement(`a`);
        a.href = url;
        a.download = `sim${simulationData.num_stocks}x${new Date().toDateString()}.json`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
    }

    _loadSimulation(file) {
        const reader = new FileReader();

        reader.onload = (e) => {
            try {
                const simulationData = JSON.parse(e.target.result);

                if (!simulationData.num_stocks || !simulationData.stockData) {
                    this._log(`Error loading simulation: missing fields`);
                    return;
                }

                if (this.startPlayback(simulationData)) {
                    this._log("Simulation loaded. Press Begin to start playback.");
                }

            } catch (err) {
                this._log(`Error loading simulation: ` + err.message);
            }
        }

        reader.onerror = () => {
            this._log('Error reading file');
        };

        reader.readAsText(file);
    }

    startPlayback(simulationData) {
        if (this.connected) {
            this._log("Disconnect from server to use playback");
            return false;
        }

        this.playbackMode = true;
        this.playbackData = simulationData;
        this.playbackIndex = 0;

        if (this.onInfoReceived) {
            this.onInfoReceived(simulationData.num_stocks, 150, this.keepCandleCount);
        }

        this._log(`Loaded simulation: ${simulationData.num_stocks} stocks`);
        return true;
    }

    playPlayback() {
        if (!this.playbackMode || this.playbackTimer) return;
        
        const intervalMs = 100 / this.playbackSpeed;
        
        this.playbackTimer = setInterval(() => {
            if (this.playbackIndex >= this._getTotalCandles()) {
                this.pausePlayback();
                this._log("Playback complete");
                return;
            }
            
            this._feedNextCandle();
            this.playbackIndex++;
        }, intervalMs);
        
        if (this.onSimulationStateChange) {
            this.onSimulationStateChange("started", true);
            this.onSimulationStateChange("paused", false);
        }
    }

    pausePlayback() {
        if (this.playbackTimer) {
            clearInterval(this.playbackTimer);
            this.playbackTimer = null;
        }
        
        if (this.onSimulationStateChange) {
            this.onSimulationStateChange("paused", true);
        }
    }

    stopPlayback() {
        this.pausePlayback();
        this.playbackMode = false;
        this.playbackData = null;
        this.playbackIndex = 0;
        
        this.clear_stocks();
        
        if (this.onSimulationStateChange) {
            this.onSimulationStateChange("started", false);
        }
    }

    _feedNextCandle() {
        for (let i = 0; i < this.playbackData.stockData.length; i++) {
            const stock = this.playbackData.stockData[i];
            
            if (this.playbackIndex < stock.candles.length) {
                const candle = stock.candles[this.playbackIndex];
                const timestamp = candle.d instanceof Date ? candle.d : new Date(candle.d);

                this.update_stock(i, timestamp, {
                    open: candle.o,
                    high: candle.h,
                    low: candle.l,
                    close: candle.c,
                    num_trades: candle.t
                });
            }
        }
    }

    _getTotalCandles() {
        if (!this.playbackData || !this.playbackData.stockData.length) return 0;
        return Math.max(...this.playbackData.stockData.map(stock => stock.candles.length));
    }

    setUpdateSpeed(speed) {
        if (this.onPlayBackSpeedChange) {
            this.onPlayBackSpeedChange(speed);
        }

        const json = this.makeCommand("set")("update_time")(`${speed}`);
        this.send_message(JSON.stringify(json));
    }

    makeCommand = (command) => {
        return (target) => {
            return (value) => {
                return {
                    "command": command,
                    "target": target,
                    "value": value
                };
            };
        }
    }

    makeCartesianProduct = () => {
        let product = [];
        for (let i = 0; i < adjectives.length - 1; i++) {
            for (let j = 0; j < nouns.length - 1; j++) {
                product.push(adjectives[i] + " " + nouns[j]);
            }
        }

        for (let i = product.length - 1; i > 0; i--) {
            const j = Math.floor(Math.random() * (i + 1));
            [product[i], product[j]] = [product[j], product[i]];
        }
        
        return product;
    }
}