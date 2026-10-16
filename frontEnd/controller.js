

class SimulationController {
    constructor() {
        this.model = new SimulationModel();
        this.view = new SimulationView();
        
        // State
        this.simulationState = {
            playing: false,
            started: false,
            terminated: false
        };

        this.userState = {
            start_funds: 0,
            user_funds: 0,
            user_profit: 0,
            user_positions: []
        };

        // Polling state
        this.currentStockIndex = 0;
        this.updateSpeed = 1000;
        this.pollingInterval = null;

        this.lastTimestamp = null
        
        // Visible Candles
        this.visibleDurationMs = 15000;
        this.extraCandleOffset = 15;
        this.displayedCandles = this.visibleDurationMs / this.updateSpeed;
        this.maxChartCandles = this.displayedCandles + this.extraCandleOffset;

        //Listeners
        this._setupModelListeners();
        this._setupViewListeners();
    }
    
    // Setup model callbacks
    _setupModelListeners() {
        this.model.onSimulationStateChange = (type, value) => {
            this._handleSimulationStateChange(type, value);
        };

        this.model.onUserStateChange = (update) => {
            this._handleUserStateChange(update);
        };

        this.model.onConnectionChange = (connected) => {
            this._handleConnectionChange(connected);
        };

        this.model.onPlayBackSpeedChange = (maxChartCandles) => {
            this._handlePlayBackSpeedChange(maxChartCandles);
        };
        
        this.model.onStockUpdate = (index, candle) => {
            this._handleStockUpdate(index, candle);
        };
        
        this.model.onStartReceived = (numStocks, initWallet, maxChartCandles) => {
            this._handleStartReceived(numStocks, initWallet, maxChartCandles);
        };

        this.model.onStartReceived = (numStocks, initWallet, time_passed) => {
            this._handleStartReceived(numStocks, initWallet, time_passed);
        }

        this.model.onUpdateReceived = (status, time_passed, stock_prices) => {
            this._handleUpdateReceived(status, time_passed, stock_prices)
        }

        this.model.onLog = (message) => {
            this.view.addLogEntry(message);
        };
    }
    
    // Setup view event listeners

    _setupViewListeners() {
        // Connection control
        this.view.onConnectClick = () => {
            if (this.model.isFirstConnect || !this.model.isConnected()) {
                this.model.connect();
            }
        };
        
        // Simulation controls
        this.view.onPlayPauseClick = () => {
            if (this.model.playbackMode) {
                if (this.model.playbackTimer) {
                    this.model.pausePlayback();
                } else {
                    this.model.playPlayback();
                }
                return;
            }

            const shouldPause = this.simulationState.playing;
            const json = this.model.makeCommand("set")("paused")(`${shouldPause ? 1 : 0}`)
            this.model.send_message(JSON.stringify(json));
        };
        
        this.view.onBeginStopClick = () => {
            if (this.model.playbackMode) {
                const isRunning = this.simulationState.started;
                
                if (isRunning) {
                    this.model.stopPlayback();
                    this.clearAllData();
                } else {
                    this.model.playPlayback();
                }
                return;
            }

            const shouldStop = this.simulationState.started;
            const json = this.model.makeCommand("set")("running")(`${shouldStop ? 0 : 1}`);
            if (shouldStop) {
                this.model.send_message(JSON.stringify(json));
                this.clearAllData();
                this._stopPolling();
            } else {
                this.model.send_message(JSON.stringify(json));
            }
        };

        this.view.onLoadSimulation = (file) => {
            this.model._loadSimulation(file);
        }
        
        this.view.onTerminateClick = () => {
            if (this.simulationState.terminated) {
                return;
            }
            const json = this.model.makeCommand("set")("should_terminate")(`${Number(!this.simulationState.terminated)}`);
            this.model.send_message(JSON.stringify(json));
        };

        this.view.onUpdateSpeedChange = (speed) => {
            if (this.model.connected) {
                this.model.setUpdateSpeed(speed);
            } else if (this.model.playbackTimer) {
                this.model.pausePlayback();
                this.model.playPlayback();
                this.view.addLogEntry(`Playback speed set to ${speed}ms`);
            }
        };

        // Config Submit
        this.view.onConfigSubmit = (command, target, value) => {
            if (target === "stats") {
                command = "local";
            }

            const json = this.model.makeCommand(command)(target)(value);

            if (command === "local") {
                this.model._handleMessage(JSON.stringify({local : target}));
            } else {
                this.model.send_message(JSON.stringify(json));
            }
        }

        // User Trade Submit
        this.view.onUserTrade = (quantity, price, side) => {
            const stock = this.currentStockIndex;

            const quantityNum = Number(quantity);
            const priceNum = Math.round(Number(price) * 100); /// changed to convert cents to dollars

            const json = {
                command: "user",
                trade: {
                    stock: stock,
                    quantity: quantityNum,
                    price: priceNum,
                    side: side
                }
            }

            //console.log(JSON.stringify(json));
            this.model.send_message(JSON.stringify(json));
        }
        
        // Stock selection
        this.view.onStockSelect = (index) => {
            this.selectStock(index);
        };

        // Playback controls
        this.view.onSaveClick = () => {
            if (this.model.num_stocks === 0 || this.model.stocks.length === 0) {
                this.view.addLogEntry("No simulation data to save");
                return;
            }
            this.model._saveSimulation();
            this.view.addLogEntry("Simulation saved");
        };

        this.view.onLoadClick = (file) => {
            if (this.model.connected && !this.model.playbackMode) {
                this.view.addLogEntry("Disconnect from server before loading simulation");
                return;
            }
            this.model._loadSimulation(file);
        };

        
        // Chart controls
        this.view.onZoom = (chart, event) => {
            event.preventDefault();

            const zoomSpeed = 0.1; // 10% per scroll
            this.visibleDurationMs *= event.deltaY > 0 ? 1 + zoomSpeed : 1 - zoomSpeed;

            this.visibleDurationMs = Math.max(1000, Math.min(this.visibleDurationMs, 60_000));

            this.displayedCandles = this.visibleDurationMs / this.updateSpeed;
            this.maxChartCandles = this.displayedCandles + this.extraCandleOffset;

            this.view.setMaxChartCandles(this.maxChartCandles);
            this.view.setVisibleDuration(this.visibleDurationMs);

            const candles = chart.data.datasets[0].data;
            if (candles.length === 0) return;

            const lastCandle = candles[candles.length - 1].x;
            const firstCandle = candles[0].x;
            const minTime = lastCandle.getTime() - this.visibleDurationMs;

            chart.options.scales.x.min = minTime > firstCandle.getTime() ? new Date(minTime) : firstCandle;
            chart.options.scales.x.max = lastCandle;

            chart.update({ duration: 200, easing: 'easeOutCubic' });
        };
    }
    
    // Handle model events

    // Server state changes
    _handleSimulationStateChange(type, value) {
        switch (type) {
            case "started":
                this.simulationState.started = value;
                this.view.updateBeginStopButton(value);
                if (value) {
                    this._startPolling();
                } else {
                    this._stopPolling();
                }
                break;
            case "paused":
                this.simulationState.playing = !value;
                this.view.updatePlayPauseButton(!value);
                if (this.simulationState.playing) {
                    this._startPolling();
                } else {
                    this._stopPolling();
                }
                break;
            case "terminated":
                this.simulationState.terminated = value;
                this.view.updateTerminateButton(value);
                if (value) {
                    this._stopPolling();
                }
                break;
        }
    }

    // User trader changes
    _handleUserStateChange({stock, quantity, side, wallet}) {
        this.userState.user_funds = wallet;
        this.userState.user_profit = this.userState.user_funds - this.userState.start_funds;

        if (side === "buy") {
            this.userState.user_positions[stock] += quantity;
        } else {
            this.userState.user_positions[stock] -= quantity;
        }

        this.view.updateUserInfo({
            funds: this.userState.user_funds,
            profit: this.userState.user_profit,
            shares: this.userState.user_positions[this.currentStockIndex]
        });
    }

    // Connection changes
    _handleConnectionChange(connected) {
        this.view.updateConnectionStatus(connected);
        
        if (connected) {
            this.simulationState.terminated = false;
            this.view.updateTerminateButton(false);
        } else {
            this._stopPolling();
        }
    }

    // Update speed changes
    _handlePlayBackSpeedChange(speed) {
        this.updateSpeed = speed;
        this.displayedCandles = this.visibleDurationMs / this.updateSpeed;
        this.maxChartCandles = this.displayedCandles + this.extraCandleOffset;

        this.view.setMaxChartCandles(this.maxChartCandles);

        if (this.simulationState.started && this.simulationState.playing) {
            this._startPolling();
        }
    }
    
    // Stock update
    _handleStockUpdate(index, candle) {
        const stock = this.model.getStock(index);
        this.view.updateStockButton(index, candle.c);
        
        if (index === this.currentStockIndex) {
            const chartCandle = {
                x: candle.d,
                o: candle.o,
                h: candle.h,
                l: candle.l,
                c: candle.c
            };
            
            this.view.addCandleToChart(chartCandle);
            
            // line graph
            if (this.view.chart.data.datasets[1]) {
                const linePoint = {x: candle.d, y: candle.c};
                this.view.chart.data.datasets[1].data.push(linePoint);
                this.view.chart.update('none');
            }
            
            this.view.updateCurrentStockInfo({
                price: null,
                volume: null,
                timestamp: candle.d
            });
        }
    }
    
    // Simulation started received
    _handleStartReceived(numStocks, initWallet, time_passed) {
        this.currentStockIndex = 0;
        this.userState.user_profit = 0;
        this.userState.start_funds = initWallet * 100;
        this.userState.user_funds = initWallet * 100;
        this.userState.user_positions = Array(numStocks).fill(10);
        
        this.simulationStartTime = Date.now() - (time_passed / 1_000_000);
        this.lastTimestamp = time_passed;

        
        let stock_names = this.model.makeCartesianProduct();
        this.model.initialize_stocks(numStocks);

        this.view.setMaxChartCandles(this.maxChartCandles);
        this.view.setVisibleDuration(this.visibleDurationMs);
        this.view.onUpdateSpeedChange(this.updateSpeed);
        this.view.initializeStockButtons(numStocks, stock_names);

        this.view.updateCurrentStockInfo({
            cents: null,
            volume: null,
            timestamp: null
        });
        this.view.updateUserInfo({
            funds: this.userState.user_funds,
            profit: this.userState.user_profit,
            shares: this.userState.user_positions[this.currentStockIndex]
        });

        this.model.requestCandles(0, this.maxChartCandles);

        this.view.addLogEntry(`Starting: stocks:${numStocks}, funds:${initWallet}`);
    }

    // Simulation update received
    _handleUpdateReceived(status, time_passed, stock_data) {
        this.lastTimestamp = time_passed;

        let prices = stock_data.prices;
        let volumes = stock_data.volumes;

        if (status == "paused") {
            return;
        }

        for (let i = 0; i < volumes.length; i++) {
            this.view.updateStockButton(i, prices[i]);

            const stock = this.model.getStock(i);
            if (stock) {
                stock.latestPrice = prices[i];
                stock.volume = volumes[i];
            }
        }

        const currentStock = this.model.getStock(this.currentStockIndex);
        if (currentStock) {
            this.view.updateCurrentStockInfo({
                cents: currentStock.latestPrice,
                volume: currentStock.volume,
                timestamp: null
            });
        }

    }

    // Request candles at an interval
    _startPolling() {
        this._stopPolling();

        if (!this.model.connected || this.model.playbackMode) return;

        this.pollingInterval = setInterval(() => {
            this.model.requestCandles(this.currentStockIndex, 1);
        }, this.updateSpeed);
    }

    // Stop requesting candles at an interval
    _stopPolling() {
        if (this.pollingInterval) {
            clearInterval(this.pollingInterval);
            this.pollingInterval = null;
        }
    }
    
    selectStock(index) {
        const stock = this.model.getStock(index);
        if (!stock) {
            console.error(`Invalid stock index: ${index}`);
            return;
        }
        
        this.currentStockIndex = index;
        this.view.setActiveStockButton(index);

        this.view.updateUserInfo({
            funds: this.userState.user_funds,
            profit: this.userState.user_profit,
            shares: this.userState.user_positions[this.currentStockIndex]
        });

        this.view.clearChart();
        
        this.view.updateCurrentStockInfo({
            cents: stock.latestPrice,
            volume: stock.volume,
            timestamp: null
        });
        
        if (this.model.connected && !this.model.playbackMode) {
            this.model.requestCandles(index, this.maxChartCandles);
        }

        if (this.simulationState.started && this.simulationState.playing) {
            this._startPolling();
        }
    }
    
    clearAllData() {
        this.model.clear_stocks();
        this.currentStockIndex = 0;

        this.userState = {
            start_funds: 0,
            user_funds: 0,
            user_profit: 0,
            user_positions: []
        };

        this.view.clearChart();
        this.view.clearStockButtons();
        this.view.updateCurrentStockInfo({
            cents: null,
            volume: null,
            timestamp: null
        });

        this.view.updateUserInfo({
            funds: null,
            profit: null,
            shares: null
        });
    }
}

// Probably redundant but ran into issues loading
document.addEventListener('DOMContentLoaded', () => {
    window.simulationController = new SimulationController();
});