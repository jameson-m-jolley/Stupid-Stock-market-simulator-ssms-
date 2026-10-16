//opening intro
let intro = document.querySelector('.intro');
let logo = document.querySelector('.logo-header');
let logoSpan = document.querySelectorAll('.logo');

window.addEventListener('DOMContentLoaded', ()=> {
    setTimeout(()=>{
        logoSpan.forEach((span, idx)=>{
            setTimeout(()=>{
                span.classList.add('active');
            },(idx + 1) * 400)
        })

        setTimeout(() => {
            logoSpan.forEach((span, idx) => {
                setTimeout(() => {
                    span.classList.remove('active');
                    span.classList.add('fade');
                }, (idx + 1) * 50);
            });
        }, 2000);

        setTimeout(() => {
            intro.style.top = '-100vh';
        }, 2300)
    
    })
})

// Chart descriptor
const chartData = {
    datasets: [{
        label: 'Stock Candles',
        data: [],
        borderColor: '#333',
        color: {
            up: '#26a69a',
            down: '#ef5350',
            unchanged: '#999'
        },

        maxBarThickness: 20
    },
        {
            label: 'Close Price Line',
            data: [],
            type: 'line',
            borderColor: '#ff9900',
            backgroundColor: 'rgba(255,153,0,0.2)',
            tension: 0.4,
            hidden: true
        }
        ]
};


// Chart config
const config = {
    type: 'candlestick',
    data: chartData,
    options: {
        responsive: false,
        scales: {
            x: {
                type: 'time',
                time: {
                    unit: 'second',
                    tooltipFormat: 'HH:mm:ss',
                    displayFormats: {
                        second: 'HH:mm:ss'
                    }
                },
                ticks: {
                    maxRotation: 0,
                    autoSkip: true,
                    maxTicksLimit: 15,
                    callback: function(value) {
                        return new Date(value).toLocaleTimeString();
                    }
                },
                grid: {
                    display: false
                }
            },
            y: {
                beginAtZero: false,
                ticks: {
                    callback: function(value) {
                        return `$${(value / 100).toFixed(2)}`;
                    },
                    autoSkip: true,
                    maxTicksLimit: 8
                },
                grid: {
                    display: false
                }
            }
        }
    }
};


// View
class SimulationView {
    constructor() {
        this._cacheElements();
        this._initializeChart();
        this._setupUIHandlers();

        this.maxChartCandles;
        this.visibleDuration;
        
        // Callbacks for controller
        this.onConnectClick = null;
        this.onPlayPauseClick = null;
        this.onBeginStopClick = null;
        this.onTerminateClick = null;
        this.onStockSelect = null;
        this.onConfigSubmit = null;
        this.onUserTrade = null;
        this.onSaveClick = null;
        this.onLoadClick = null;
        this.onUpdateSpeedChange = null;
        this.onZoom = null;
    }
    
    _cacheElements() {
        // Control buttons
        this.connectBtn = document.getElementById('connect');
        this.playBtn = document.getElementById('play-pause');
        this.beginBtn = document.getElementById('begin-end');
        this.terminateBtn = document.getElementById('terminate');
        this.mixedSelect = document.getElementById('mixed');

        // Config form
        this.configForm = document.getElementById('config-form');

        // User info displays
        this.walletEl = document.getElementById('current-funds');
        this.profitEl = document.getElementById('current-profit');
        this.sharesEl = document.getElementById('current-shares');

        // User form
        this.userForm = document.getElementById('user-form');
        
        // Stock info displays
        this.priceEl = document.getElementById('current-price');
        this.tradesEl = document.getElementById('number-of-trades');
        this.candleEl = document.getElementById('most-recent-candle');
        
        // Containers
        this.logContainer = document.getElementById('log_container');
        this.stockList = document.getElementById('stock-list');
        this.chartEl = document.getElementById('myChart');
        
        // Panel controls
        this.hamburgerBtn = document.getElementById('hamburger-btn');
        this.simPanel = document.getElementById('sim-panel');
        this.closeBtn = document.getElementById('close-panel');

        // Playback controls
        this.saveSimBtn = document.getElementById('save-sim');
        this.loadSimInput = document.getElementById('load-sim');
        this.updateSpeedSlider = document.getElementById('playback-speed');
        this.speedDisplay = document.getElementById('speed-display');
    }
    
    _initializeChart() {
        const ctx = this.chartEl.getContext('2d');
        this.chart = new Chart(ctx, config);
    }
    
    _setupUIHandlers() {
        // Hamburger menu
        this.hamburgerBtn.addEventListener('click', () => {
            this.simPanel.classList.add('open');
        });
        
        this.closeBtn.addEventListener('click', () => {
            this.simPanel.classList.remove('open');
        });

        // Chart control
        this.chartEl.addEventListener('wheel', (event) => {
            event.preventDefault();
            if (this.onZoom) {
                this.onZoom(this.chart, event);
            }
        })
        
        // Control buttons
        this.connectBtn.addEventListener('click', () => {
            if (this.onConnectClick) this.onConnectClick();
        });
        
        this.playBtn.addEventListener('click', () => {
            if (this.onPlayPauseClick) this.onPlayPauseClick();
        });
        
        this.beginBtn.addEventListener('click', () => {
            if (this.onBeginStopClick) this.onBeginStopClick();
        });
        
        this.terminateBtn.addEventListener('click', () => {
            if (this.onTerminateClick) this.onTerminateClick();
        });

        // Config form
        this.configForm.addEventListener('submit', (e) => {
            e.preventDefault();
            const formData = new FormData(this.configForm);

            if (this.onConfigSubmit) {
                this.onConfigSubmit(
                    formData.get('command'),
                    formData.get('target'),
                    formData.get('value')
                )
            }
        })

        // User form
        this.userForm.addEventListener('submit', (e) => {
            e.preventDefault();
            const formData = new FormData(this.userForm);

            if (this.onUserTrade) {
                this.onUserTrade(
                    formData.get('quantity'),
                    formData.get('price'),
                    formData.get('side')
                )
            }
        })

        // Playback controls
        this.saveSimBtn.addEventListener('click', () => {
            if (this.onSaveClick) this.onSaveClick();
        });

        this.loadSimInput.addEventListener('change', (e) => {
            const file = e.target.files[0];
            if (file && this.onLoadClick) {
                this.onLoadClick(file);
            }
        });

        this.updateSpeedSlider.addEventListener('input', (e) => {
            const speed = parseFloat(e.target.value);
            this.speedDisplay.textContent = `${speed} ms`;
            if (this.onUpdateSpeedChange) {
                this.onUpdateSpeedChange(speed);
            }
        });

        this.mixedSelect.addEventListener('change', () => {
            const mixed = this.mixedSelect.value;

            if (mixed === 'true') {
                this.chart.data.datasets[1].hidden = false;
            } else {
                this.chart.data.datasets[1].hidden = true;
            }

            this.chart.update();
        });
            
    }

    // Computes the Y steps to fit the deviation in set steps
    _computeVolatilityStep() {
        const candles = this.chart.data.datasets[0].data;
        if (!candles || candles.length < 2) return 1;

        let sum = 0, sumSq = 0, n = candles.length;

        for (let i = 0; i < n; i++) {
            const c = candles[i].c;
            sum += c;
            sumSq += c * c;
        }

        const mean = sum / n;
        const variance = sumSq / n - mean * mean;
        const stddev = Math.sqrt(Math.max(variance, 0));

        let step = stddev * 0.5;
        if (step < 1) step = 1;

        const niceSteps = [1, 2, 5, 10, 25, 50, 100, 250, 500];

        let best = niceSteps[0];
        let bestDiff = Math.abs(best - step);

        for (let i = 1; i < niceSteps.length; i++) {
            const diff = Math.abs(niceSteps[i] - step);
            if (diff < bestDiff) {
                best = niceSteps[i];
                bestDiff = diff;
            }
        }

        return best;
    }
    
    // Public methods for controller to call
    
    // Log display
    addLogEntry(message) {
        if (!message) return;
        
        const entry = document.createElement('div');
        entry.classList.add('log-entry');
        entry.textContent = message;
        
        this.logContainer.appendChild(entry);
        this.logContainer.scrollTop = this.logContainer.scrollHeight;
    }
    
    // Connection status
    updateConnectionStatus(connected) {
        if (connected) {
            this.connectBtn.classList.remove('terminated');
            this.connectBtn.classList.add('active');
            this.connectBtn.textContent = 'Connected';
        } else {
            this.connectBtn.classList.remove('active');
            this.connectBtn.classList.add('terminated');
            this.connectBtn.textContent = 'Reconnect';
        }
    }

    // moved to bottom div
    updatePlayPauseButton(playing) {
        this.playBtn.textContent = playing ? '❚❚' : '►';
    }


    updateBeginStopButton(started) {
        this.beginBtn.textContent = started ? 'Stop' : 'Begin';
    }
    
    // hamburger
    updateTerminateButton(terminated) {
        if (terminated) {
            this.terminateBtn.classList.add('terminated');
            this.terminateBtn.textContent = 'TERMINATED';
        } else {
            this.terminateBtn.classList.remove('terminated');
            this.terminateBtn.textContent = 'Terminate';
        }
    }
    
    // Stock info display
    updateCurrentStockInfo({cents, volume, timestamp }) {
        if (cents !== null && cents !== undefined) {
            this.priceEl.textContent = this._formatPrice(cents);
        }

        if (volume !== null && volume !== undefined) {
            this.tradesEl.textContent = volume;
        }

        if (timestamp !== null && timestamp !== undefined) {
            this.candleEl.textContent = this._formatTime(timestamp);
        }
    }

    // User info display
    updateUserInfo({funds, profit, shares}) {
        this.walletEl.textContent = funds != null ? this._formatPrice(funds) : '—';
        this.profitEl.textContent = profit != null ? this._formatPrice(profit) : '—';
        this.sharesEl.textContent = shares != null ? shares : '—';
    }
    
    // Stock buttons
    initializeStockButtons(numStocks, stock_names) {
        this.stockList.innerHTML = '';
        
        for (let i = 0; i < numStocks; i++) {
            const li = document.createElement('li');
            const button = document.createElement('button');
            button.className = 'stock-button';
            button.dataset.index = i;
            
            if (i === 0) {
                button.classList.add('active');
            }
            
            const nameSpan = document.createElement('span');
            nameSpan.className = 'name';

            if (i < stock_names.length - 1) {
                nameSpan.textContent = `${stock_names[i]}: `;
            } else {
                nameSpan.textContent = `Stock ${i} `;
            }

            const priceSpan = document.createElement('span');
            priceSpan.className = 'price';
            priceSpan.textContent = '—';
            
            button.appendChild(nameSpan);
            button.appendChild(priceSpan);
            
            button.addEventListener('click', () => {
                if (this.onStockSelect) {
                    this.onStockSelect(i);
                }
            });
            
            li.appendChild(button);
            this.stockList.appendChild(li);
        }
    }
    
    updateStockButton(index, cents) {
        const button = this.stockList.querySelector(`button[data-index="${index}"]`);
        if (button) {
            const priceSpan = button.querySelector('.price');
            if (priceSpan) {
                priceSpan.textContent = this._formatPrice(cents);
            }
        }
    }
    
    setActiveStockButton(index) {
        const buttons = this.stockList.querySelectorAll('.stock-button');
        buttons.forEach(btn => btn.classList.remove('active'));
        
        const activeButton = this.stockList.querySelector(`button[data-index="${index}"]`);
        if (activeButton) {
            activeButton.classList.add('active');
        }
    }
    
    clearStockButtons() {
        this.stockList.innerHTML = '';
    }

    setMaxChartCandles(maxChartCandles) {
        this.maxChartCandles = maxChartCandles;
    }

    setVisibleDuration(durationMs) {
        this.visibleDuration = durationMs;
    }
    
    // Chart methods
    setChartData(candleData, maxChartCandles) {
        this.maxChartCandles = maxChartCandles;
        this.chart.data.datasets[0].data = candleData;

        this.chart.data.datasets[1].data = candleData.map(c => ({
            x: c.x,
            y: c.c
        }));

        this.chart.update();
    }
    
    addCandleToChart(candle) {
        if (!(candle.x instanceof Date) || Number.isNaN(candle.x.getTime())) return;

        this.chart.data.datasets[0].data.push(candle);

        while (this.chart.data.datasets[0].data.length > this.maxChartCandles) {
            this.chart.data.datasets[0].data.shift();
            //console.log(`cleaning up stored candles to below window size (${this.maxChartCandles})`);
        }

        if (this.chart.data.datasets[0].data.length > 0) {
            const firstCandle = this.chart.data.datasets[0].data[0].x;
            const lastCandle = this.chart.data.datasets[0].data[this.chart.data.datasets[0].data.length - 1].x;

            const minTime = lastCandle.getTime() - this.visibleDuration;

            this.chart.options.scales.x.min = minTime > firstCandle.getTime()
                ? new Date(minTime)
                : firstCandle;

            this.chart.options.scales.x.max = lastCandle;
        }

        // compute new Y step
        const step = this._computeVolatilityStep();
        this.chart.options.scales.y.ticks.stepSize = step;


        this.chart.update();
    }

    clearChart() {
        this.chart.data.datasets[0].data = [];

        if (this.chart.data.datasets[1]) {
            this.chart.data.datasets[1].data = [];
        }

        this.chart.options.scales.x.min = undefined;
        this.chart.options.scales.x.max = undefined;

        this.chart.update('none');
    }
    
    // Formatting helpers
    _formatPrice(price) {
        return `$${(Number(price) / 100).toFixed(2)}`;
    }
    
    _formatTime(date) {
        return date instanceof Date ? date.toLocaleTimeString() : date;
    }
}