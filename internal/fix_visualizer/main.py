import socket
import struct
import time
import random
import threading
from dataclasses import dataclass, field
from enum import IntEnum
from collections import defaultdict
from datetime import datetime, UTC
import math
import tkinter as tk
from tkinter import ttk
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
import sys
import traceback
from queue import Queue, Empty

SOH = "\x01"

class Instrument(IntEnum):
    EURUSD = 0
    GBPUSD = 1
    USDJPY = 2
    AUDUSD = 3
    USDCAD = 4
    COUNT = 5

    @staticmethod
    def from_string(s):
        try:
            return Instrument[s]
        except KeyError:
            print(f"Warning: Unknown instrument string '{s}', defaulting to EURUSD.")
            return Instrument.EURUSD

    def to_string(self):
        return self.name

class OrderSide(IntEnum):
    BUY = 1
    SELL = 2

@dataclass
class Order:
    symbol: Instrument
    quantity: int
    price: float
    side: OrderSide
    cl_ord_id: str = ""
    trader_id: str = ""

@dataclass
class MDEntry:
    md_update_action: int
    md_entry_type: int
    md_entry_px: float
    md_entry_size: int
    symbol: Instrument

@dataclass
class SnapshotData:
    symbol: Instrument
    entries: list[MDEntry]

@dataclass
class IncrementalData:
    symbol: Instrument
    entry: MDEntry

class OrderGenerator:
    def __init__(self, buy_trader_pool, sell_trader_pool):
        self.buy_trader_pool = buy_trader_pool
        self.sell_trader_pool = sell_trader_pool
        self.mean_price = 1.2000
        self.reversion_speed = 0.05
        self.base_vol = 0.002
        self.vol_cluster_factor = 0.3
        self.last_price = {inst: self.mean_price for inst in Instrument if inst != Instrument.COUNT}
        self.inst_vol = {inst: self.base_vol for inst in Instrument if inst != Instrument.COUNT}

    def generate_order(self):
        inst = Instrument(random.randint(0, Instrument.COUNT - 1))
        self.inst_vol[inst] = (self.inst_vol[inst] * (1.0 - self.vol_cluster_factor) + self.base_vol * self.vol_cluster_factor)
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
        
        if side == OrderSide.BUY:
            trader = random.choice(self.buy_trader_pool)
        else: # SELL
            trader = random.choice(self.sell_trader_pool)
            
        return Order(inst, qty, new_price, side, trader_id=trader)

class OrderBook:
    def __init__(self):
        self.bids = defaultdict(dict)
        self.asks = defaultdict(dict)
        self.trades = defaultdict(list)
        self.price_history = defaultdict(list)
        self.ohlc_data = defaultdict(lambda: {'timestamps': [], 'open': [], 'high': [], 'low': [], 'close': [], 'volume': []})
        self.current_candle = defaultdict(lambda: {'open': None, 'high': 0, 'low': float('inf'), 'close': 0, 'volume': 0, 'start_time': time.time()})

    def apply_snapshot(self, symbol, entries):
        self.bids[symbol].clear()
        self.asks[symbol].clear()
        for entry in entries:
            if entry.md_entry_type == 0:
                self.bids[symbol][entry.md_entry_px] = entry.md_entry_size
            elif entry.md_entry_type == 1:
                self.asks[symbol][entry.md_entry_px] = entry.md_entry_size

    def apply_incremental(self, symbol, entry):
        target_book = self.bids[symbol] if entry.md_entry_type == 0 else self.asks[symbol]
        if entry.md_update_action == 0:
            target_book[entry.md_entry_px] = entry.md_entry_size
        elif entry.md_update_action == 1:
            target_book[entry.md_entry_px] = entry.md_entry_size
        elif entry.md_update_action == 2:
            if entry.md_entry_px in target_book:
                del target_book[entry.md_entry_px]

    def update_trade(self, symbol: Instrument, price: float, qty: int):
        now = datetime.now()
        self.trades[symbol].append((now, price, qty))
        self.price_history[symbol].append((time.time(), price))
        if len(self.trades[symbol]) > 1000: self.trades[symbol] = self.trades[symbol][-1000:]
        if len(self.price_history[symbol]) > 1000: self.price_history[symbol] = self.price_history[symbol][-1000:]

        candle = self.current_candle[symbol]
        if candle['open'] is None:
            candle['open'] = price
            candle['start_time'] = time.time()
        candle['high'] = max(candle['high'], price)
        candle['low'] = min(candle['low'], price)
        candle['close'] = price
        candle['volume'] += qty

        if time.time() - candle['start_time'] > 5.0:
            ohlc = self.ohlc_data[symbol]
            ohlc['timestamps'].append(now)
            ohlc['open'].append(candle['open'])
            ohlc['high'].append(candle['high'])
            ohlc['low'].append(candle['low'])
            ohlc['close'].append(candle['close'])
            ohlc['volume'].append(candle['volume'])
            if len(ohlc['timestamps']) > 100:
                for key in ohlc: ohlc[key] = ohlc[key][-100:]
            candle.update({'open': None, 'high': 0, 'low': float('inf'), 'close': 0, 'volume': 0, 'start_time': time.time()})

    def get_top_of_book(self, symbol: Instrument, depth=10):
        bids = sorted(self.bids[symbol].items(), key=lambda x: x[0], reverse=True)[:depth]
        asks = sorted(self.asks[symbol].items(), key=lambda x: x[0])[:depth]
        return bids, asks

class TextRedirector:
    def __init__(self, widget, tag="stdout"):
        self.widget = widget
        self.tag = tag

    def write(self, str):
        self.widget.configure(state="normal")
        self.widget.insert("end", str, (self.tag,))
        self.widget.see("end")
        self.widget.configure(state="disabled")

    def flush(self):
        pass

class TradingGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("FIX Trading Simulator - Live Dashboard")
        self.root.geometry("1600x900")
        self.root.configure(bg='#1e1e1e')
        self.client = None
        self.selected_instrument = Instrument.EURUSD
        self.data_queue = Queue() # FIX: Create queue for thread-safe data passing
        self.setup_ui()

    def setup_ui(self):
        control_frame = tk.Frame(self.root, bg='#2d2d2d', height=60)
        control_frame.pack(fill=tk.X, padx=5, pady=5)
        control_frame.pack_propagate(False)
        tk.Label(control_frame, text="FIX Trading Simulator", font=('Arial', 16, 'bold'), bg='#2d2d2d', fg='#00ff00').pack(side=tk.LEFT, padx=10)
        self.start_btn = tk.Button(control_frame, text="▶ START", font=('Arial', 12, 'bold'), bg='#00aa00', fg='white', command=self.start_trading, width=10)
        self.start_btn.pack(side=tk.LEFT, padx=5)
        self.stop_btn = tk.Button(control_frame, text="⏹ STOP", font=('Arial', 12, 'bold'), bg='#aa0000', fg='white', command=self.stop_trading, width=10, state=tk.DISABLED)
        self.stop_btn.pack(side=tk.LEFT, padx=5)
        self.status_label = tk.Label(control_frame, text="⚫ Disconnected", font=('Arial', 11), bg='#2d2d2d', fg='#ff0000')
        self.status_label.pack(side=tk.RIGHT, padx=10)

        main_frame = tk.Frame(self.root, bg='#1e1e1e')
        main_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        left_panel = tk.Frame(main_frame, bg='#2d2d2d', width=400)
        left_panel.pack(side=tk.LEFT, fill=tk.BOTH, padx=(0, 5))
        left_panel.pack_propagate(False)

        stats_frame = tk.LabelFrame(left_panel, text="📊 STATISTICS", font=('Arial', 11, 'bold'), bg='#2d2d2d', fg='#00ff00', bd=2)
        stats_frame.pack(fill=tk.X, padx=5, pady=5)
        self.stats_text = tk.Text(stats_frame, height=8, bg='#1e1e1e', fg='#00ff00', font=('Consolas', 10), relief=tk.FLAT)
        self.stats_text.pack(fill=tk.BOTH, padx=5, pady=5)

        selector_frame = tk.LabelFrame(left_panel, text="🎯 SELECT INSTRUMENT", font=('Arial', 11, 'bold'), bg='#2d2d2d', fg='#00ff00', bd=2)
        selector_frame.pack(fill=tk.X, padx=5, pady=5)
        for inst in Instrument:
            if inst != Instrument.COUNT:
                btn = tk.Button(selector_frame, text=inst.name, font=('Arial', 10, 'bold'), bg='#3d3d3d', fg='#ffffff', command=lambda i=inst: self.select_instrument(i))
                btn.pack(fill=tk.X, padx=5, pady=2)

        book_frame = tk.LabelFrame(left_panel, text="📖 ORDER BOOK", font=('Arial', 11, 'bold'), bg='#2d2d2d', fg='#00ff00', bd=2)
        book_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        self.book_text = tk.Text(book_frame, bg='#1e1e1e', fg='#ffffff', font=('Consolas', 9), relief=tk.FLAT)
        self.book_text.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        right_panel = tk.Frame(main_frame, bg='#2d2d2d')
        right_panel.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)

        chart_frame = tk.Frame(right_panel, bg='#2d2d2d')
        chart_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 5))

        self.fig = Figure(figsize=(12, 6), facecolor='#1e1e1e')
        self.ax_ohlc = self.fig.add_subplot(211, facecolor='#1e1e1e')
        self.ax_volume = self.fig.add_subplot(212, facecolor='#1e1e1e', sharex=self.ax_ohlc)
        self.fig.tight_layout()
        self.canvas = FigureCanvasTkAgg(self.fig, master=chart_frame)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

        log_frame = tk.LabelFrame(right_panel, text="📜 FIX LOGS", font=('Arial', 11, 'bold'), bg='#2d2d2d', fg='#00ff00', bd=2)
        log_frame.pack(fill=tk.X, pady=(5, 0))
        
        self.log_text = tk.Text(log_frame, height=10, bg='#1e1e1e', fg='#cccccc', font=('Consolas', 9), relief=tk.FLAT)
        self.log_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=5, pady=5)
        log_scrollbar = ttk.Scrollbar(log_frame, command=self.log_text.yview)
        log_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.log_text.config(yscrollcommand=log_scrollbar.set)
        self.log_text.configure(state="disabled")

    def select_instrument(self, instrument):
        self.selected_instrument = instrument
        if self.client and self.client.running:
            self.client.send_market_data_request(instrument)

    def start_trading(self):
        self.start_btn.config(state=tk.DISABLED)
        self.stop_btn.config(state=tk.NORMAL)
        self.status_label.config(text="🟡 Connecting...", fg='#ffff00')
        
        self.log_text.configure(state="normal")
        self.log_text.delete('1.0', tk.END)
        self.log_text.configure(state="disabled")

        sys.stdout = TextRedirector(self.log_text, "stdout")
        sys.stderr = TextRedirector(self.log_text, "stderr")
        
        self.client = FIXClient(gui=self, data_queue=self.data_queue)
        threading.Thread(target=self.client.run, daemon=True).start()
        self.update_gui()

    def stop_trading(self):
        if self.client:
            self.client.stop()
        self.start_btn.config(state=tk.NORMAL)
        self.stop_btn.config(state=tk.DISABLED)
        self.status_label.config(text="⚫ Disconnected", fg='#ff0000')
        sys.stdout = sys.__stdout__
        sys.stderr = sys.__stderr__

    def process_queue(self):
        try:
            while True:
                data = self.data_queue.get_nowait()
                if isinstance(data, SnapshotData):
                    self.client.order_book.apply_snapshot(data.symbol, data.entries)
                elif isinstance(data, IncrementalData):
                    self.client.order_book.apply_incremental(data.symbol, data.entry)
                    if data.entry.md_entry_type == 2: # Trade
                        self.client.order_book.update_trade(data.symbol, data.entry.md_entry_px, data.entry.md_entry_size)
        except Empty:
            pass

    def update_gui(self):
        if self.client and self.client.running:
            self.process_queue() # FIX: Process data from the queue
            self.update_stats()
            self.update_order_book()
            self.update_charts()
            self.root.after(100, self.update_gui) # Update more frequently

    def update_stats(self):
        if not self.client: return
        elapsed = (datetime.now() - self.client.start_time).total_seconds() if self.client.start_time else 0
        rate = self.client.orders_sent / elapsed if elapsed > 0 else 0
        fill_rate = (self.client.fills_received / self.client.orders_sent * 100) if self.client.orders_sent > 0 else 0
        stats = f"""Runtime:            {elapsed:.1f}s
Orders Sent:        {self.client.orders_sent}
Executions Rcvd:    {self.client.executions_received}
Fills Rcvd:         {self.client.fills_received}
Total Filled Qty:   {self.client.total_filled_qty}
Order Rate:         {rate:.2f} orders/sec
Fill Rate:          {fill_rate:.1f}%"""
        self.stats_text.delete('1.0', tk.END)
        self.stats_text.insert('1.0', stats)

    def update_order_book(self):
        if not self.client: return
        bids, asks = self.client.order_book.get_top_of_book(self.selected_instrument, depth=15)
        book_text = f"{'='*50}\n  {self.selected_instrument.name} ORDER BOOK\n{'='*50}\n\n{'BIDS':^25} | {'ASKS':^25}\n{'-'*25}-+-{'-'*25}\n"
        for i in range(max(len(bids), len(asks))):
            bid_str = f"{bids[i][0]:.5f} ({bids[i][1]:,})" if i < len(bids) else ""
            ask_str = f"{asks[i][0]:.5f} ({asks[i][1]:,})" if i < len(asks) else ""
            book_text += f"{bid_str:>25} | {ask_str:<25}\n"
        self.book_text.delete('1.0', tk.END)
        self.book_text.insert('1.0', book_text)

    def update_charts(self):
        if not self.client: return
        ohlc = self.client.order_book.ohlc_data[self.selected_instrument]
        if len(ohlc['timestamps']) < 2: return
        self.ax_ohlc.clear()
        self.ax_volume.clear()
        for i in range(len(ohlc['timestamps'])):
            x, o, h, l, c = i, ohlc['open'][i], ohlc['high'][i], ohlc['low'][i], ohlc['close'][i]
            color = '#00ff00' if c >= o else '#ff0000'
            self.ax_ohlc.plot([x, x], [l, h], color=color, linewidth=1)
            self.ax_ohlc.add_patch(plt.Rectangle((x - 0.3, min(o, c)), 0.6, abs(c - o), facecolor=color, edgecolor=color))
        self.ax_ohlc.set_title(f'{self.selected_instrument.name} OHLC', color='#00ff00', fontsize=14, fontweight='bold')
        self.ax_ohlc.tick_params(axis='y', colors='#00ff00')
        self.ax_ohlc.tick_params(axis='x', colors='#1e1e1e')
        self.ax_ohlc.grid(True, alpha=0.2)
        colors = ['#00ff00' if ohlc['close'][i] >= ohlc['open'][i] else '#ff0000' for i in range(len(ohlc['timestamps']))]
        self.ax_volume.bar(range(len(ohlc['volume'])), ohlc['volume'], color=colors, width=0.8)
        self.ax_volume.set_title('Volume', color='#00ff00', fontsize=12)
        self.ax_volume.tick_params(colors='#00ff00')
        self.ax_volume.grid(True, alpha=0.2)
        self.fig.tight_layout()
        self.canvas.draw()

class FIXClient:
    def __init__(self, host='127.0.0.1', port=8088, gui=None, data_queue=None):
        self.host, self.port, self.gui, self.data_queue = host, port, gui, data_queue
        self.socket, self.connected = None, False
        
        self.buy_traders = [f"BUYER_{i}" for i in range(1, 11)]
        self.sell_traders = [f"SELLER_{i}" for i in range(1, 11)]
        self.order_generator = OrderGenerator(self.buy_traders, self.sell_traders)
        
        self.order_book = OrderBook()
        self.orders_sent, self.executions_received, self.fills_received, self.total_filled_qty = 0, 0, 0, 0
        self.start_time, self.running = None, False
        self.msg_seq_num = 1
        self.cl_ord_id_counter = 1
        self.target_comp_id = "TRADING_CORE"
        self.receive_buffer = b""

    def _calculate_checksum(self, body_str):
        return sum(body_str.encode('ascii')) % 256

    def _build_fix_message(self, msg_type, sender_comp_id, body_tags):
        body_parts = []
        body_parts.append(f"35={msg_type}")
        body_parts.append(f"49={sender_comp_id}")
        body_parts.append(f"56={self.target_comp_id}")
        body_parts.append(f"34={self.msg_seq_num}")
        body_parts.append(f"52={datetime.now(UTC).strftime('%Y%m%d-%H:%M:%S.%f')[:-3]}")
        
        for tag, value in body_tags.items():
            if isinstance(value, list):
                for item in value:
                    body_parts.append(f"{tag}={item}")
            else:
                body_parts.append(f"{tag}={value}")

        body_str = SOH.join(body_parts) + SOH
        
        header_str = f"8=FIXT.1.1{SOH}9={len(body_str)}{SOH}"
        
        final_msg = header_str + body_str
        checksum = self._calculate_checksum(final_msg)
        final_msg += f"10={checksum:03}{SOH}"
        
        self.msg_seq_num += 1
        return final_msg.encode('ascii')

    def _parse_fix_message(self, msg_str):
        msg_dict = defaultdict(list)
        try:
            fields = msg_str.strip().split(SOH)
            for field in fields:
                if '=' in field:
                    tag, value = field.split('=', 1)
                    tag = int(tag)
                    msg_dict[tag].append(value)
            
            final_dict = {}
            for tag, values in msg_dict.items():
                if len(values) == 1:
                    final_dict[tag] = values[0]
                else:
                    final_dict[tag] = values
            return final_dict

        except Exception as e:
            print(f"Error parsing FIX message: {e}, message: '{msg_str}'")
            return None

    def connect(self):
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            self.connected = True
            print(f"✓ Connected to FIX server at {self.host}:{self.port}")
            self.gui.status_label.config(text="🟢 Connected", fg='#00ff00')
            return True
        except Exception as e:
            print(f"✗ Connection failed: {e}")
            self.gui.status_label.config(text="🔴 Connection Failed", fg='#ff0000')
            return False

    def send_logon(self):
        print("→ Sending Logon (35=A)")
        logon_msg = self._build_fix_message('A', "VISUALIZER_SESSION", {98: 0, 108: 30, 141: 'Y'})
        self.socket.sendall(logon_msg)

    def send_logout(self):
        if not self.connected: return
        print("→ Sending Logout (35=5)")
        logout_msg = self._build_fix_message('5', "VISUALIZER_SESSION", {})
        self.socket.sendall(logout_msg)

    def send_market_data_request(self, instrument):
        md_req_id = f"MD_{instrument.name}_{int(time.time())}"
        print(f"→ Sending Market Data Request (35=V) for {instrument.name}")
        
        tags = {
            262: md_req_id,
            263: 1,
            264: 0,
            267: 2,
            269: ['0', '1'],
            146: 1,
            55: instrument.to_string()
        }
        md_msg = self._build_fix_message('V', "VISUALIZER_SESSION", tags)
        self.socket.sendall(md_msg)

    def send_new_order_single(self):
        if not self.connected: return
        order = self.order_generator.generate_order()
        
        order.cl_ord_id = str(self.cl_ord_id_counter)
        self.cl_ord_id_counter += 1
        
        order_tags = {
            11: order.cl_ord_id,
            55: order.symbol.to_string(),
            54: order.side.value,
            60: datetime.now(UTC).strftime("%Y%m%d-%H:%M:%S.%f")[:-3],
            38: order.quantity,
            40: 2,
            44: f"{order.price:.5f}"
        }
        order_msg = self._build_fix_message('D', order.trader_id, order_tags)
        try:
            self.socket.sendall(order_msg)
            self.orders_sent += 1
        except Exception as e:
            print(f"✗ Error sending order: {e}")
            self.connected = False

    def _receive_loop(self):
        while self.running:
            try:
                data = self.socket.recv(4096)
                if not data:
                    print("✗ Server closed connection.")
                    self.connected = False
                    break
                self.receive_buffer += data
                
                while SOH.encode('ascii') in self.receive_buffer:
                    start_index = self.receive_buffer.find(b'8=FIXT.1.1' + SOH.encode('ascii'))
                    if start_index == -1: break

                    body_len_start = self.receive_buffer.find(b'9=', start_index)
                    if body_len_start == -1: break
                    
                    body_len_end = self.receive_buffer.find(SOH.encode('ascii'), body_len_start)
                    if body_len_end == -1: break

                    body_len_val_str = self.receive_buffer[body_len_start + 2 : body_len_end]
                    body_len = int(body_len_val_str)

                    body_start_index = body_len_end + 1
                    end_of_message_index = body_start_index + body_len + 7
                    
                    if len(self.receive_buffer) >= end_of_message_index:
                        msg_bytes = self.receive_buffer[start_index:end_of_message_index]
                        self._handle_message(msg_bytes.decode('ascii'))
                        self.receive_buffer = self.receive_buffer[end_of_message_index:]
                    else:
                        break

            except socket.timeout:
                continue
            except Exception:
                print("✗ Error in receive loop:")
                traceback.print_exc()
                break
        self.connected = False

    def _handle_message(self, msg_str):
        print(f"← RAW: {msg_str.replace(SOH, '|')}")
        msg = self._parse_fix_message(msg_str)
        if not msg or 35 not in msg: return

        msg_type = msg[35]
        if msg_type == '8': self._handle_execution_report(msg)
        elif msg_type == 'W': self._handle_market_data_snapshot(msg)
        elif msg_type == 'X': self._handle_market_data_incremental(msg)
        elif msg_type == 'j': print(f"Business Message Reject: {msg.get(58, 'No text')}")
        elif msg_type == '3': print(f"Session Level Reject: {msg.get(58, 'No text')}")

    def _handle_execution_report(self, msg):
        print(f"Parsed ExecutionReport (35=8): {msg}")
        self.executions_received += 1
        if msg.get(39) in ['1', '2', 'F']:
            self.fills_received += 1
            self.total_filled_qty += int(msg.get(32, 0))

    def _handle_market_data_snapshot(self, msg):
        symbol = Instrument.from_string(msg[55])
        print(f"Parsed Market Data Snapshot (35=W) for {symbol.name}: {msg}")
        
        md_entries = self._parse_repeating_group(msg, 268, [269, 270, 271, 290])
        
        processed_entries = []
        for entry_group in md_entries:
            processed_entries.append(MDEntry(
                md_update_action=0,
                md_entry_type=int(entry_group[269]),
                md_entry_px=float(entry_group[270]),
                md_entry_size=int(entry_group[271]),
                symbol=symbol
            ))
        print(f"  > Queueing {len(processed_entries)} entries for {symbol.name} snapshot")
        self.data_queue.put(SnapshotData(symbol, processed_entries))

    def _handle_market_data_incremental(self, msg):
        if 55 not in msg:
            print("Warning: MarketDataIncrementalRefresh missing symbol (55). Skipping.")
            return
        symbol = Instrument.from_string(msg[55])
        print(f"Parsed Market Data Incremental (35=X) for {symbol.name}: {msg}")

        md_entries = self._parse_repeating_group(msg, 268, [279, 269, 270, 271])
        
        for entry_group in md_entries:
            entry = MDEntry(
                md_update_action=int(entry_group[279]),
                md_entry_type=int(entry_group[269]),
                md_entry_px=float(entry_group[270]),
                md_entry_size=int(entry_group[271]),
                symbol=symbol
            )
            print(f"  > Queueing incremental update for {symbol.name}: {entry}")
            self.data_queue.put(IncrementalData(symbol, entry))

    def _parse_repeating_group(self, msg, group_count_tag, field_tags):
        groups = []
        try:
            num_in_group = int(msg.get(group_count_tag, 0))
            if num_in_group == 0:
                return groups

            for tag in field_tags:
                if tag in msg and not isinstance(msg[tag], list):
                    msg[tag] = [msg[tag]]

            for i in range(num_in_group):
                entry = {}
                is_entry_valid = True
                for tag in field_tags:
                    if tag in msg and i < len(msg[tag]):
                        entry[tag] = msg[tag][i]
                    else:
                        is_entry_valid = False
                        break
                if is_entry_valid:
                    groups.append(entry)
        except Exception as e:
            print(f"Error parsing repeating group for tag {group_count_tag}: {e}")
            traceback.print_exc()
            
        return groups

    def run(self):
        if not self.connect():
            self.stop()
            return

        self.running = True
        self.start_time = datetime.now()
        self.socket.settimeout(1.0)

        self.send_logon()
        time.sleep(0.1)
        self.send_market_data_request(self.gui.selected_instrument)

        receiver_thread = threading.Thread(target=self._receive_loop, daemon=True)
        receiver_thread.start()

        order_send_interval = 0.1
        while self.running and self.connected:
            self.send_new_order_single()
            time.sleep(order_send_interval)

    def stop(self):
        if not self.running: return
        self.running = False
        if self.connected and self.socket:
            try:
                self.send_logout()
                time.sleep(0.5)
            except Exception as e:
                print(f"Exception during logout: {e}")
            finally:
                self.socket.close()
        self.connected = False
        print("Client stopped.")

if __name__ == "__main__":
    root = tk.Tk()
    app = TradingGUI(root)
    def on_closing():
        if app.client:
            app.client.stop()
        root.destroy()
    root.protocol("WM_DELETE_WINDOW", on_closing)
    root.mainloop()
