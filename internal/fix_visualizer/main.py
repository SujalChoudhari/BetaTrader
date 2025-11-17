import socket
import struct
import time
import random
import threading
from dataclasses import dataclass
from enum import IntEnum
from collections import defaultdict, deque
from datetime import datetime
import math
import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import matplotlib.animation as animation

class Instrument(IntEnum):
    EURUSD = 0
    GBPUSD = 1
    USDJPY = 2
    AUDUSD = 3
    USDCAD = 4
    COUNT = 5

class OrderSide(IntEnum):
    BUY = 0
    SELL = 1

@dataclass
class Order:
    symbol: Instrument
    quantity: int
    price: float
    side: OrderSide

@dataclass
class Execution:
    order_id: str
    symbol: Instrument
    filled_qty: int
    price: float
    status: int
    timestamp: datetime

class OrderGenerator:
    def __init__(self):
        self.mean_price = 1.2000
        self.reversion_speed = 0.05
        self.base_vol = 0.002
        self.vol_cluster_factor = 0.3

        self.last_price = {}
        self.inst_vol = {}

        for inst in Instrument:
            if inst != Instrument.COUNT:
                self.last_price[inst] = self.mean_price
                self.inst_vol[inst] = self.base_vol

    def generate_order(self):
        inst = Instrument(random.randint(0, Instrument.COUNT - 1))

        self.inst_vol[inst] = (self.inst_vol[inst] * (1.0 - self.vol_cluster_factor) +
                               self.base_vol * self.vol_cluster_factor)

        if random.random() < 0.005:
            self.inst_vol[inst] *= 3.0

        shock = random.gauss(0.0, self.inst_vol[inst])
        reversion = self.reversion_speed * (self.mean_price - self.last_price[inst])
        new_price = self.last_price[inst] * math.exp(shock) + reversion
        new_price = max(0.0001, new_price)

        self.last_price[inst] = new_price

        qty = int(random.lognormvariate(4.0, 1.0))
        qty = max(1, min(5000, qty))

        side = OrderSide.BUY if random.random() < 0.5 else OrderSide.SELL

        return Order(inst, qty, new_price, side)

class OrderBook:
    def __init__(self):
        self.bids = defaultdict(lambda: defaultdict(int))
        self.asks = defaultdict(lambda: defaultdict(int))
        self.trades = defaultdict(list)
        self.price_history = defaultdict(list)
        self.ohlc_data = defaultdict(lambda: {'timestamps': [], 'open': [], 'high': [], 'low': [], 'close': [], 'volume': []})
        self.current_candle = defaultdict(lambda: {'open': None, 'high': 0, 'low': float('inf'), 'close': 0, 'volume': 0, 'start_time': time.time()})

    def add_order(self, order: Order):
        if order.side == OrderSide.BUY:
            self.bids[order.symbol][order.price] += order.quantity
        else:
            self.asks[order.symbol][order.price] += order.quantity

    def update_trade(self, symbol: Instrument, price: float, qty: int):
        now = datetime.now()
        self.trades[symbol].append((now, price, qty))
        self.price_history[symbol].append((time.time(), price))

        # Keep only last 1000 trades
        if len(self.trades[symbol]) > 1000:
            self.trades[symbol] = self.trades[symbol][-1000:]
        if len(self.price_history[symbol]) > 1000:
            self.price_history[symbol] = self.price_history[symbol][-1000:]

        # Update current candle
        candle = self.current_candle[symbol]
        if candle['open'] is None:
            candle['open'] = price
            candle['start_time'] = time.time()
        candle['high'] = max(candle['high'], price)
        candle['low'] = min(candle['low'], price)
        candle['close'] = price
        candle['volume'] += qty

        # Create new candle every 5 seconds
        if time.time() - candle['start_time'] > 5.0:
            ohlc = self.ohlc_data[symbol]
            ohlc['timestamps'].append(now)
            ohlc['open'].append(candle['open'])
            ohlc['high'].append(candle['high'])
            ohlc['low'].append(candle['low'])
            ohlc['close'].append(candle['close'])
            ohlc['volume'].append(candle['volume'])

            # Keep only last 100 candles
            if len(ohlc['timestamps']) > 100:
                for key in ['timestamps', 'open', 'high', 'low', 'close', 'volume']:
                    ohlc[key] = ohlc[key][-100:]

            # Reset candle
            candle['open'] = None
            candle['high'] = 0
            candle['low'] = float('inf')
            candle['close'] = 0
            candle['volume'] = 0
            candle['start_time'] = time.time()

    def get_top_of_book(self, symbol: Instrument, depth=10):
        bids = sorted(self.bids[symbol].items(), key=lambda x: x[0], reverse=True)[:depth]
        asks = sorted(self.asks[symbol].items(), key=lambda x: x[0])[:depth]
        return bids, asks

class TradingGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("FIX Trading Simulator - Live Dashboard")
        self.root.geometry("1600x900")
        self.root.configure(bg='#1e1e1e')

        self.client = None
        self.selected_instrument = Instrument.EURUSD

        self.setup_ui()

    def setup_ui(self):
        # Top Control Panel
        control_frame = tk.Frame(self.root, bg='#2d2d2d', height=60)
        control_frame.pack(fill=tk.X, padx=5, pady=5)
        control_frame.pack_propagate(False)

        tk.Label(control_frame, text="FIX Trading Simulator", font=('Arial', 16, 'bold'),
                 bg='#2d2d2d', fg='#00ff00').pack(side=tk.LEFT, padx=10)

        self.start_btn = tk.Button(control_frame, text="▶ START", font=('Arial', 12, 'bold'),
                                   bg='#00aa00', fg='white', command=self.start_trading, width=10)
        self.start_btn.pack(side=tk.LEFT, padx=5)

        self.stop_btn = tk.Button(control_frame, text="⏹ STOP", font=('Arial', 12, 'bold'),
                                  bg='#aa0000', fg='white', command=self.stop_trading,
                                  width=10, state=tk.DISABLED)
        self.stop_btn.pack(side=tk.LEFT, padx=5)

        self.status_label = tk.Label(control_frame, text="⚫ Disconnected", font=('Arial', 11),
                                     bg='#2d2d2d', fg='#ff0000')
        self.status_label.pack(side=tk.RIGHT, padx=10)

        # Main Content Area
        main_frame = tk.Frame(self.root, bg='#1e1e1e')
        main_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Left Panel - Stats & Order Book
        left_panel = tk.Frame(main_frame, bg='#2d2d2d', width=400)
        left_panel.pack(side=tk.LEFT, fill=tk.BOTH, padx=(0, 5))
        left_panel.pack_propagate(False)

        # Statistics
        stats_frame = tk.LabelFrame(left_panel, text="📊 STATISTICS", font=('Arial', 11, 'bold'),
                                    bg='#2d2d2d', fg='#00ff00', bd=2)
        stats_frame.pack(fill=tk.X, padx=5, pady=5)

        self.stats_text = tk.Text(stats_frame, height=8, bg='#1e1e1e', fg='#00ff00',
                                  font=('Consolas', 10), relief=tk.FLAT)
        self.stats_text.pack(fill=tk.BOTH, padx=5, pady=5)

        # Instrument Selector
        selector_frame = tk.LabelFrame(left_panel, text="🎯 SELECT INSTRUMENT", font=('Arial', 11, 'bold'),
                                       bg='#2d2d2d', fg='#00ff00', bd=2)
        selector_frame.pack(fill=tk.X, padx=5, pady=5)

        for inst in Instrument:
            if inst != Instrument.COUNT:
                btn = tk.Button(selector_frame, text=inst.name, font=('Arial', 10, 'bold'),
                                bg='#3d3d3d', fg='#ffffff', command=lambda i=inst: self.select_instrument(i))
                btn.pack(fill=tk.X, padx=5, pady=2)

        # Order Book
        book_frame = tk.LabelFrame(left_panel, text="📖 ORDER BOOK", font=('Arial', 11, 'bold'),
                                   bg='#2d2d2d', fg='#00ff00', bd=2)
        book_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        self.book_text = tk.Text(book_frame, bg='#1e1e1e', fg='#ffffff',
                                 font=('Consolas', 9), relief=tk.FLAT)
        self.book_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Right Panel - Charts
        right_panel = tk.Frame(main_frame, bg='#2d2d2d')
        right_panel.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)

        # Create matplotlib figure
        self.fig = Figure(figsize=(12, 8), facecolor='#1e1e1e')

        # OHLC Chart
        self.ax_ohlc = self.fig.add_subplot(211, facecolor='#1e1e1e')
        self.ax_ohlc.set_title('OHLC Candlestick Chart', color='#00ff00', fontsize=14, fontweight='bold')
        self.ax_ohlc.tick_params(colors='#00ff00')

        # Volume Chart
        self.ax_volume = self.fig.add_subplot(212, facecolor='#1e1e1e')
        self.ax_volume.set_title('Volume', color='#00ff00', fontsize=12)
        self.ax_volume.tick_params(colors='#00ff00')

        self.fig.tight_layout()

        self.canvas = FigureCanvasTkAgg(self.fig, master=right_panel)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

    def select_instrument(self, instrument):
        self.selected_instrument = instrument

    def start_trading(self):
        self.start_btn.config(state=tk.DISABLED)
        self.stop_btn.config(state=tk.NORMAL)
        self.status_label.config(text="🟢 Connected", fg='#00ff00')

        # Create and start client
        self.client = FIXClient(gui=self)
        threading.Thread(target=self.client.run, daemon=True).start()

        # Start GUI update loop
        self.update_gui()

    def stop_trading(self):
        if self.client:
            self.client.running = False
        self.start_btn.config(state=tk.NORMAL)
        self.stop_btn.config(state=tk.DISABLED)
        self.status_label.config(text="⚫ Disconnected", fg='#ff0000')

    def update_gui(self):
        if self.client and self.client.running:
            self.update_stats()
            self.update_order_book()
            self.update_charts()
            self.root.after(500, self.update_gui)

    def update_stats(self):
        if not self.client:
            return

        elapsed = (datetime.now() - self.client.start_time).total_seconds() if self.client.start_time else 0
        rate = self.client.orders_sent / elapsed if elapsed > 0 else 0
        fill_rate = (self.client.executions_received / self.client.orders_sent * 100) if self.client.orders_sent > 0 else 0

        stats = f"""
Runtime:            {elapsed:.1f}s
Orders Sent:        {self.client.orders_sent}
Executions:         {self.client.executions_received}
Total Filled Qty:   {self.client.total_filled_qty}
Order Rate:         {rate:.2f} orders/sec
Fill Rate:          {fill_rate:.1f}%
"""
        self.stats_text.delete('1.0', tk.END)
        self.stats_text.insert('1.0', stats)

    def update_order_book(self):
        if not self.client:
            return

        bids, asks = self.client.order_book.get_top_of_book(self.selected_instrument, depth=15)

        book_text = f"{'='*50}\n"
        book_text += f"  {self.selected_instrument.name} ORDER BOOK\n"
        book_text += f"{'='*50}\n\n"
        book_text += f"{'BIDS':^25} | {'ASKS':^25}\n"
        book_text += f"{'-'*25}-+-{'-'*25}\n"

        max_len = max(len(bids), len(asks))
        for i in range(max_len):
            bid_str = f"{bids[i][0]:.5f} ({bids[i][1]:,})" if i < len(bids) else ""
            ask_str = f"{asks[i][0]:.5f} ({asks[i][1]:,})" if i < len(asks) else ""
            book_text += f"{bid_str:>25} | {ask_str:<25}\n"

        self.book_text.delete('1.0', tk.END)
        self.book_text.insert('1.0', book_text)

    def update_charts(self):
        if not self.client:
            return

        ohlc = self.client.order_book.ohlc_data[self.selected_instrument]

        if len(ohlc['timestamps']) < 2:
            return

        # Clear axes
        self.ax_ohlc.clear()
        self.ax_volume.clear()

        # Plot candlesticks
        for i in range(len(ohlc['timestamps'])):
            x = i
            o, h, l, c = ohlc['open'][i], ohlc['high'][i], ohlc['low'][i], ohlc['close'][i]

            color = '#00ff00' if c >= o else '#ff0000'

            # Candle body
            self.ax_ohlc.plot([x, x], [l, h], color=color, linewidth=1)
            body_height = abs(c - o)
            body_bottom = min(o, c)
            self.ax_ohlc.add_patch(plt.Rectangle((x - 0.3, body_bottom), 0.6, body_height,
                                                 facecolor=color, edgecolor=color))

        self.ax_ohlc.set_title(f'{self.selected_instrument.name} OHLC', color='#00ff00',
                               fontsize=14, fontweight='bold')
        self.ax_ohlc.tick_params(colors='#00ff00')
        self.ax_ohlc.grid(True, alpha=0.2)

        # Plot volume
        colors = ['#00ff00' if ohlc['close'][i] >= ohlc['open'][i] else '#ff0000'
                  for i in range(len(ohlc['timestamps']))]
        self.ax_volume.bar(range(len(ohlc['volume'])), ohlc['volume'], color=colors, width=0.8)
        self.ax_volume.set_title('Volume', color='#00ff00', fontsize=12)
        self.ax_volume.tick_params(colors='#00ff00')
        self.ax_volume.grid(True, alpha=0.2)

        self.fig.tight_layout()
        self.canvas.draw()

class FIXClient:
    def __init__(self, host='127.0.0.1', port=12345, gui=None):
        self.host = host
        self.port = port
        self.socket = None
        self.connected = False
        self.order_generator = OrderGenerator()
        self.order_book = OrderBook()
        self.gui = gui

        self.orders_sent = 0
        self.executions_received = 0
        self.total_filled_qty = 0
        self.start_time = None
        self.running = False

    def connect(self):
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            self.connected = True
            print(f"✓ Connected to FIX server at {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"✗ Connection failed: {e}")
            return False

    def serialize_order(self, order: Order) -> bytes:
        data = struct.pack('<IIdI',
                           order.symbol.value,
                           order.quantity,
                           order.price,
                           order.side.value)
        return data

    def deserialize_execution(self, data: bytes) -> Execution:
        if len(data) >= 28:
            order_id, symbol, filled_qty, price, status = struct.unpack('<QIIdI', data[:28])
            return Execution(str(order_id), Instrument(symbol), filled_qty, price, status, datetime.now())
        return None

    def send_order(self):
        if not self.connected:
            return

        order = self.order_generator.generate_order()

        try:
            data = self.serialize_order(order)
            self.socket.sendall(data)
            self.orders_sent += 1
            self.order_book.add_order(order)
        except Exception as e:
            print(f"✗ Error sending order: {e}")
            self.connected = False

    def receive_executions(self):
        buffer = b''

        while self.running:
            try:
                data = self.socket.recv(4096)
                if not data:
                    self.connected = False
                    break

                buffer += data

                while len(buffer) >= 28:
                    exec_data = buffer[:28]
                    buffer = buffer[28:]

                    execution = self.deserialize_execution(exec_data)
                    if execution:
                        self.executions_received += 1
                        self.total_filled_qty += execution.filled_qty
                        self.order_book.update_trade(execution.symbol, execution.price, execution.filled_qty)

            except socket.timeout:
                continue
            except Exception as e:
                break

    def run(self):
        if not self.connect():
            return

        self.running = True
        self.start_time = datetime.now()
        self.socket.settimeout(0.1)

        receiver_thread = threading.Thread(target=self.receive_executions, daemon=True)
        receiver_thread.start()

        interval = 0.01  # 10ms

        while self.running and self.connected:
            self.send_order()
            time.sleep(interval)

        if self.socket:
            self.socket.close()

if __name__ == "__main__":
    root = tk.Tk()
    app = TradingGUI(root)
    root.mainloop()