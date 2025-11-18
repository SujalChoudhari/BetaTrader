"""
FIX Trading Simulator - Refactored
A clean, modular implementation of a FIX protocol trading simulator with GUI
"""

import socket
import struct
import time
import random
import threading
from dataclasses import dataclass, field
from enum import IntEnum
from collections import defaultdict, deque
from datetime import datetime, timezone
from typing import Dict, List, Tuple, Optional, Any
from queue import Queue, Empty
import math
import traceback
import sys

import tkinter as tk
from tkinter import ttk
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.patches as mpatches


# ============================================================================
# CONSTANTS
# ============================================================================

SOH = "\x01"
DEFAULT_HOST = '127.0.0.1'
DEFAULT_PORT = 8088
TARGET_COMP_ID = "TRADING_CORE"
SENDER_COMP_ID = "VISUALIZER_SESSION"


# ============================================================================
# ENUMS & DATA CLASSES
# ============================================================================

class Instrument(IntEnum):
    """Trading instruments enumeration"""
    EURUSD = 0
    GBPUSD = 1
    USDJPY = 2
    AUDUSD = 3
    USDCAD = 4
    COUNT = 5

    @staticmethod
    def from_string(s: str) -> 'Instrument':
        """Convert string to Instrument enum"""
        try:
            return Instrument[s]
        except (KeyError, ValueError):
            return Instrument.EURUSD

    def to_string(self) -> str:
        """Convert Instrument to string"""
        return self.name


class OrderSide(IntEnum):
    """Order side enumeration"""
    BUY = 1
    SELL = 2


@dataclass
class Order:
    """Represents a trading order"""
    symbol: Instrument
    quantity: int
    price: float
    side: OrderSide
    cl_ord_id: str = ""
    trader_id: str = ""


@dataclass
class MDEntry:
    """Market data entry"""
    md_update_action: int
    md_entry_type: int
    md_entry_px: float
    md_entry_size: int
    symbol: Optional[Instrument] = None


@dataclass
class SnapshotData:
    """Market data snapshot"""
    symbol: Instrument
    entries: List[MDEntry]


@dataclass
class IncrementalData:
    """Incremental market data update"""
    symbol: Instrument
    entry: MDEntry


# ============================================================================
# ORDER GENERATION
# ============================================================================

class OrderGenerator:
    """Generates random trading orders with realistic price dynamics"""

    def __init__(self, buy_trader_pool: List[str], sell_trader_pool: List[str]):
        self.buy_trader_pool = buy_trader_pool
        self.sell_trader_pool = sell_trader_pool

        # Price model parameters
        self.mean_price = 80.400
        self.reversion_speed = 0.09
        self.base_vol = 0.02
        self.vol_cluster_factor = 0.3
        self.volatility_spike_probability = 0.05
        self.volatility_spike_multiplier = 3.0

        # Per-instrument state
        self.last_price: Dict[Instrument, float] = {
            inst: self.mean_price
            for inst in Instrument
            if inst != Instrument.COUNT
        }
        self.inst_vol: Dict[Instrument, float] = {
            inst: self.base_vol
            for inst in Instrument
            if inst != Instrument.COUNT
        }

    def generate_order(self) -> Order:
        """Generate a random order with realistic pricing"""
        # Select random instrument
        inst = Instrument(random.randint(0, Instrument.COUNT - 1))

        # Update volatility with clustering effect
        self.inst_vol[inst] = (
                self.inst_vol[inst] * (1.0 - self.vol_cluster_factor) +
                self.base_vol * self.vol_cluster_factor
        )

        # Occasional volatility spikes
        if random.random() < self.volatility_spike_probability:
            self.inst_vol[inst] *= self.volatility_spike_multiplier

        # Mean reversion with volatility
        shock = random.gauss(0.0, self.inst_vol[inst])
        reversion = self.reversion_speed * (self.mean_price - self.last_price[inst])
        new_price = self.last_price[inst] * math.exp(shock) + reversion
        new_price = max(0.0001, new_price)

        self.last_price[inst] = new_price

        # Generate quantity (log-normal distribution)
        qty = int(random.lognormvariate(4.0, 1.0))
        qty = max(1, min(5000, qty))

        # Random side and trader
        side = OrderSide.BUY if random.random() < 0.5 else OrderSide.SELL
        trader_pool = self.buy_trader_pool if side == OrderSide.BUY else self.sell_trader_pool
        trader = random.choice(trader_pool)

        return Order(
            symbol=inst,
            quantity=qty,
            price=new_price,
            side=side,
            trader_id=trader
        )


# ============================================================================
# ORDER BOOK
# ============================================================================

@dataclass
class Candle:
    """OHLC candle data"""
    open: Optional[float] = None
    high: float = 0
    low: float = float('inf')
    close: float = 0
    volume: int = 0
    start_time: float = field(default_factory=time.time)


class OrderBook:
    """Manages order book state and market data"""

    MAX_TRADES = 1000
    MAX_PRICE_HISTORY = 1000
    MAX_CANDLES = 100
    CANDLE_INTERVAL = 1.0  # seconds

    def __init__(self):
        self.bids: Dict[Instrument, Dict[float, int]] = defaultdict(dict)
        self.asks: Dict[Instrument, Dict[float, int]] = defaultdict(dict)
        self.trades: Dict[Instrument, deque] = defaultdict(lambda: deque(maxlen=self.MAX_TRADES))
        self.price_history: Dict[Instrument, deque] = defaultdict(lambda: deque(maxlen=self.MAX_PRICE_HISTORY))

        # OHLC data
        self.ohlc_data: Dict[Instrument, Dict[str, List]] = defaultdict(
            lambda: {
                'timestamps': [],
                'open': [],
                'high': [],
                'low': [],
                'close': [],
                'volume': []
            }
        )
        self.current_candle: Dict[Instrument, Candle] = defaultdict(Candle)

    def apply_snapshot(self, symbol: Instrument, entries: List[MDEntry]) -> None:
        """Apply a full order book snapshot"""
        self.bids[symbol].clear()
        self.asks[symbol].clear()

        for entry in entries:
            if entry.md_entry_type == 0:  # Bid
                self.bids[symbol][float(entry.md_entry_px)] = int(entry.md_entry_size)
            elif entry.md_entry_type == 1:  # Ask
                self.asks[symbol][float(entry.md_entry_px)] = int(entry.md_entry_size)

    def apply_incremental(self, symbol: Instrument, entry: MDEntry) -> None:
        """Apply an incremental order book update"""
        if entry.md_entry_type == 2:  # Trade
            return

        target_book = self.bids[symbol] if entry.md_entry_type == 0 else self.asks[symbol]
        price = float(entry.md_entry_px)

        if entry.md_update_action in (0, 1):  # New or Change
            target_book[price] = int(entry.md_entry_size)
        elif entry.md_update_action == 2:  # Delete
            target_book.pop(price, None)

    def update_trade(self, symbol: Instrument, price: float, qty: int) -> None:
        """Record a trade and update OHLC data"""
        now = datetime.now()

        # Record trade
        self.trades[symbol].append((now, price, qty))
        self.price_history[symbol].append((time.time(), price))

        # Update current candle
        candle = self.current_candle[symbol]
        if candle.open is None:
            candle.open = price
            candle.start_time = time.time()
            print(f"[ORDERBOOK] Started new candle for {symbol.name} at {price}")

        candle.high = max(candle.high, price)
        candle.low = min(candle.low, price)
        candle.close = price
        candle.volume += qty

        # Close candle if interval elapsed
        if time.time() - candle.start_time > self.CANDLE_INTERVAL:
            self._close_candle(symbol, now, candle)
            print(f"[ORDERBOOK] Closed candle for {symbol.name}: O={candle.open:.5f} H={candle.high:.5f} L={candle.low:.5f} C={candle.close:.5f} V={candle.volume}")

    def _close_candle(self, symbol: Instrument, timestamp: datetime, candle: Candle) -> None:
        """Close the current candle and start a new one"""
        ohlc = self.ohlc_data[symbol]

        ohlc['timestamps'].append(timestamp)
        ohlc['open'].append(candle.open)
        ohlc['high'].append(candle.high)
        ohlc['low'].append(candle.low)
        ohlc['close'].append(candle.close)
        ohlc['volume'].append(candle.volume)

        # Trim old data
        if len(ohlc['timestamps']) > self.MAX_CANDLES:
            for key in ohlc:
                ohlc[key] = ohlc[key][-self.MAX_CANDLES:]

        # Reset candle
        self.current_candle[symbol] = Candle()

    def get_top_of_book(self, symbol: Instrument, depth: int = 10) -> Tuple[List, List]:
        """Get top N levels of the order book"""
        bids = sorted(self.bids[symbol].items(), key=lambda x: x[0], reverse=True)[:depth]
        asks = sorted(self.asks[symbol].items(), key=lambda x: x[0])[:depth]
        return bids, asks


# ============================================================================
# FIX PROTOCOL CLIENT
# ============================================================================

class FIXMessageBuilder:
    """Handles FIX message construction"""

    @staticmethod
    def calculate_checksum(body_str: str) -> int:
        """Calculate FIX message checksum"""
        return sum(body_str.encode('ascii')) % 256

    @staticmethod
    def build_message(
            msg_type: str,
            sender_comp_id: str,
            target_comp_id: str,
            msg_seq_num: int,
            body_tags: Dict[int, Any]
    ) -> bytes:
        """Build a complete FIX message"""
        body_parts = [
            f"35={msg_type}",
            f"49={sender_comp_id}",
            f"56={target_comp_id}",
            f"34={msg_seq_num}",
            f"52={datetime.now(timezone.utc).strftime('%Y%m%d-%H:%M:%S.%f')[:-3]}"
        ]

        # Add custom tags
        for tag, value in body_tags.items():
            if isinstance(value, list):
                body_parts.extend(f"{tag}={item}" for item in value)
            else:
                body_parts.append(f"{tag}={value}")

        body_str = SOH.join(body_parts) + SOH
        header_str = f"8=FIXT.1.1{SOH}9={len(body_str)}{SOH}"

        full_message = header_str + body_str
        checksum = FIXMessageBuilder.calculate_checksum(full_message)
        full_message += f"10={checksum:03}{SOH}"

        return full_message.encode('ascii')


class FIXMessageParser:
    """Handles FIX message parsing"""

    @staticmethod
    def parse_message(msg_str: str) -> Optional[Dict[int, Any]]:
        """Parse a FIX message string into a dictionary"""
        try:
            fields = msg_str.strip().split(SOH)
            msg_dict = defaultdict(list)

            for field in fields:
                if '=' not in field:
                    continue

                tag_str, value = field.split('=', 1)
                try:
                    tag = int(tag_str)
                    msg_dict[tag].append(value)
                except ValueError:
                    continue

            # Flatten single-value lists
            return {
                tag: values[0] if len(values) == 1 else values
                for tag, values in msg_dict.items()
            }
        except Exception:
            return None

    @staticmethod
    def parse_repeating_group(
            msg: Dict[int, Any],
            group_count_tag: int,
            field_tags: List[int]
    ) -> List[Dict[int, str]]:
        """Parse a repeating group from a FIX message"""
        groups = []

        try:
            num_in_group = int(msg.get(group_count_tag, 0))
            if num_in_group == 0:
                return groups

            # Collect all values for each field tag
            tag_values = {}
            for tag in field_tags:
                val = msg.get(tag)
                if val is None:
                    tag_values[tag] = []
                elif isinstance(val, list):
                    tag_values[tag] = val
                else:
                    tag_values[tag] = [val]

            # Build group entries
            for i in range(num_in_group):
                entry = {}
                for tag in field_tags:
                    values = tag_values.get(tag, [])
                    if i < len(values):
                        entry[tag] = values[i]
                    else:
                        print(f"Warning: incomplete repeating-group entry at index {i}")
                        break
                else:
                    groups.append(entry)

        except Exception as e:
            print(f"Error parsing repeating group for tag {group_count_tag}: {e}")

        return groups


class FIXClient:
    """FIX protocol client for trading operations"""

    def __init__(
            self,
            host: str = DEFAULT_HOST,
            port: int = DEFAULT_PORT,
            gui: Optional['TradingGUI'] = None,
            data_queue: Optional[Queue] = None
    ):
        self.host = host
        self.port = port
        self.gui = gui
        self.data_queue = data_queue

        # Network
        self.socket: Optional[socket.socket] = None
        self.connected = False
        self.receive_buffer = b""
        self.socket_timeout = 1.0

        # Traders and order generation
        self.buy_traders = [f"BUYER_{i}" for i in range(1, 11)]
        self.sell_traders = [f"SELLER_{i}" for i in range(1, 11)]
        self.order_generator = OrderGenerator(self.buy_traders, self.sell_traders)

        # State
        self.order_book = OrderBook()
        self.orders_sent = 0
        self.executions_received = 0
        self.fills_received = 0
        self.total_filled_qty = 0
        self.start_time: Optional[datetime] = None
        self.running = False

        # FIX protocol
        self.msg_seq_num = 1
        self.cl_ord_id_counter = 1
        self.target_comp_id = TARGET_COMP_ID

    def connect(self) -> bool:
        """Establish connection to FIX server"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            self.connected = True

            if self.gui:
                self.gui.status_label.config(text="🟢 Connected", fg='#00ff00')

            return True

        except Exception as e:
            if self.gui:
                self.gui.status_label.config(text="🔴 Connection Failed", fg='#ff0000')
            print(f"Connection error: {e}")
            return False

    def send_logon(self) -> None:
        """Send FIX Logon message"""
        logon_msg = FIXMessageBuilder.build_message(
            msg_type='A',
            sender_comp_id=SENDER_COMP_ID,
            target_comp_id=self.target_comp_id,
            msg_seq_num=self.msg_seq_num,
            body_tags={98: 0, 108: 30, 141: 'Y'}
        )
        self.msg_seq_num += 1

        try:
            self.socket.sendall(logon_msg)
            print("→ Sent Logon")
        except Exception as e:
            print(f"Error sending logon: {e}")

    def send_logout(self) -> None:
        """Send FIX Logout message"""
        if not self.connected:
            return

        logout_msg = FIXMessageBuilder.build_message(
            msg_type='5',
            sender_comp_id=SENDER_COMP_ID,
            target_comp_id=self.target_comp_id,
            msg_seq_num=self.msg_seq_num,
            body_tags={}
        )
        self.msg_seq_num += 1

        try:
            self.socket.sendall(logout_msg)
            print("→ Sent Logout")
        except Exception as e:
            print(f"Error sending logout: {e}")

    def send_market_data_request(self, instrument: Instrument) -> None:
        """Request market data for an instrument"""
        md_req_id = f"MD_{instrument.name}_{int(time.time())}"

        md_msg = FIXMessageBuilder.build_message(
            msg_type='V',
            sender_comp_id=SENDER_COMP_ID,
            target_comp_id=self.target_comp_id,
            msg_seq_num=self.msg_seq_num,
            body_tags={
                262: md_req_id,
                263: 1,  # Subscription
                264: 0,  # Full refresh
                267: 2,  # Number of MDEntryTypes
                269: ['0', '1'],  # Bid and Ask
                146: 1,  # Number of symbols
                55: instrument.to_string()
            }
        )
        self.msg_seq_num += 1

        try:
            self.socket.sendall(md_msg)
            print(f"→ SENT: Market Data Request for {instrument.name}")
        except Exception as e:
            print(f"ERROR sending market data request: {e}")

    def send_new_order_single(self) -> None:
        """Send a new order"""
        if not self.connected:
            return

        order = self.order_generator.generate_order()
        order.cl_ord_id = str(self.cl_ord_id_counter)
        self.cl_ord_id_counter += 1

        order_msg = FIXMessageBuilder.build_message(
            msg_type='D',
            sender_comp_id=order.trader_id,
            target_comp_id=self.target_comp_id,
            msg_seq_num=self.msg_seq_num,
            body_tags={
                11: order.cl_ord_id,
                55: order.symbol.to_string(),
                54: order.side.value,
                60: datetime.now(timezone.utc).strftime("%Y%m%d-%H:%M:%S.%f")[:-3],
                38: order.quantity,
                40: 2,  # Limit order
                44: f"{order.price:.5f}"
            }
        )
        self.msg_seq_num += 1

        try:
            self.socket.sendall(order_msg)
            self.orders_sent += 1
        except Exception as e:
            print(f"Error sending order: {e}")
            self.connected = False

    def _receive_loop(self) -> None:
        """Background thread for receiving messages"""
        while self.running:
            try:
                data = self.socket.recv(4096)
                if not data:
                    self.connected = False
                    break

                self.receive_buffer += data
                self._process_buffer()

            except socket.timeout:
                continue
            except Exception as e:
                print(f"Receive error: {e}")
                traceback.print_exc()
                break

        self.connected = False

    def _process_buffer(self) -> None:
        """Process received data buffer for complete FIX messages"""
        while True:
            # Look for message start
            start_index = self.receive_buffer.find(b'8=')
            if start_index == -1:
                break

            # Find body length field
            body_len_start = self.receive_buffer.find(b'9=', start_index)
            if body_len_start == -1:
                break

            body_len_end = self.receive_buffer.find(SOH.encode('ascii'), body_len_start)
            if body_len_end == -1:
                break

            # Parse body length
            try:
                body_len_str = self.receive_buffer[body_len_start + 2:body_len_end]
                body_len = int(body_len_str)
            except ValueError:
                self.receive_buffer = self.receive_buffer[body_len_end + 1:]
                continue

            # Check if we have the complete message
            body_start = body_len_end + 1
            message_end = body_start + body_len + 7  # +7 for checksum field

            if len(self.receive_buffer) >= message_end:
                msg_bytes = self.receive_buffer[start_index:message_end]
                try:
                    self._handle_message(msg_bytes.decode('ascii'))
                except Exception as e:
                    print(f"Error handling message: {e}")
                    traceback.print_exc()

                self.receive_buffer = self.receive_buffer[message_end:]
            else:
                break

    def _handle_message(self, msg_str: str) -> None:
        """Handle a received FIX message"""
        msg = FIXMessageParser.parse_message(msg_str)
        if not msg or 35 not in msg:
            return

        msg_type = msg[35]

        # Only print every 10th execution report to reduce lag
        if msg_type == '8':
            if self.executions_received % 10 == 0:
                print(f"← Exec #{self.executions_received}: {msg.get(55, 'N/A')} {msg.get(39, 'N/A')}")
        else:
            # Always print non-execution messages (market data, etc)
            print(f"← {msg_type}: {msg_str.replace(SOH, '|')[:120]}...")

        handlers = {
            '8': self._handle_execution_report,
            'W': self._handle_market_data_snapshot,
            'X': self._handle_market_data_incremental,
            'j': lambda m: print(f"Business Message Reject: {m.get(58, 'No text')}"),
            '3': lambda m: print(f"Session Level Reject: {m.get(58, 'No text')}")
        }

        handler = handlers.get(msg_type)
        if handler:
            handler(msg)

    def _handle_execution_report(self, msg: Dict[int, Any]) -> None:
        """Handle execution report message"""
        self.executions_received += 1

        exec_status = msg.get(39)
        if exec_status in ['1', '2', 'F']:  # Partially filled, Filled, or Trade
            self.fills_received += 1
            self.total_filled_qty += int(msg.get(32, 0))

    def _handle_market_data_snapshot(self, msg: Dict[int, Any]) -> None:
        """Handle market data snapshot message"""
        symbol = Instrument.from_string(msg.get(55, ''))

        md_entries = FIXMessageParser.parse_repeating_group(msg, 268, [269, 270, 271, 290])

        print(f"  → Processing snapshot for {symbol.name}: {len(md_entries)} entries")

        processed_entries = [
            MDEntry(
                md_update_action=0,
                md_entry_type=int(entry[269]),
                md_entry_px=float(entry[270]),
                md_entry_size=int(entry[271]),
                symbol=symbol
            )
            for entry in md_entries
        ]

        if self.data_queue:
            self.data_queue.put(SnapshotData(symbol, processed_entries))
            print(f"  → Queued snapshot data for {symbol.name}")

    def _handle_market_data_incremental(self, msg: Dict[int, Any]) -> None:
        """Handle incremental market data update"""
        if 55 not in msg:
            return

        symbol = Instrument.from_string(msg[55])
        md_entries = FIXMessageParser.parse_repeating_group(msg, 268, [279, 269, 270, 271])

        # print(f"  → Processing {len(md_entries)} incremental updates for {symbol.name}")

        for entry_group in md_entries:
            entry = MDEntry(
                md_update_action=int(entry_group[279]),
                md_entry_type=int(entry_group[269]),
                md_entry_px=float(entry_group[270]),
                md_entry_size=int(entry_group[271]),
                symbol=symbol
            )

            # Debug trades specifically
            if entry.md_entry_type == 2:
                print(f"  → [TRADE] {symbol.name}: {entry.md_entry_px:.5f} x {entry.md_entry_size}")

            if self.data_queue:
                self.data_queue.put(IncrementalData(symbol, entry))

    def run(self) -> None:
        """Main client run loop"""
        if not self.connect():
            self.stop()
            return

        self.running = True
        self.start_time = datetime.now()
        self.socket.settimeout(self.socket_timeout)

        # Send initial messages
        self.send_logon()
        time.sleep(0.1)

        if self.gui:
            self.send_market_data_request(self.gui.selected_instrument)

        # Start receiver thread
        receiver_thread = threading.Thread(target=self._receive_loop, daemon=True)
        receiver_thread.start()

        # Order sending loop
        order_send_interval = 0.1
        while self.running and self.connected:
            self.send_new_order_single()
            time.sleep(order_send_interval)

    def stop(self) -> None:
        """Stop the client"""
        if not self.running:
            return

        self.running = False

        if self.connected and self.socket:
            try:
                self.send_logout()
                time.sleep(0.5)
            except Exception:
                pass
            finally:
                try:
                    self.socket.close()
                except Exception:
                    pass

        self.connected = False
        print("Client stopped.")


# ============================================================================
# GUI COMPONENTS
# ============================================================================

class TextRedirector:
    """Redirects stdout/stderr to a tkinter Text widget"""

    def __init__(self, widget: tk.Text, tag: str = "stdout"):
        self.widget = widget
        self.tag = tag
        self.buffer = []
        self.last_flush = time.time()
        self.flush_interval = 0.5  # Flush every 500ms

    def write(self, text: str) -> None:
        """Write text to the widget (buffered)"""
        if not text:
            return

        self.buffer.append(str(text))

        # Auto-flush if buffer is large or time elapsed
        if len(self.buffer) > 20 or (time.time() - self.last_flush) > self.flush_interval:
            self.flush()

    def flush(self) -> None:
        """Flush buffered text to widget"""
        if not self.buffer:
            return

        try:
            self.widget.configure(state="normal")
            combined_text = ''.join(self.buffer)
            self.widget.insert("end", combined_text, (self.tag,))

            # Keep only last 5000 characters to prevent memory issues
            content = self.widget.get("1.0", "end")
            if len(content) > 5000:
                self.widget.delete("1.0", f"1.{len(content)-5000}")

            self.widget.see("end")
            self.widget.configure(state="disabled")

            self.buffer = []
            self.last_flush = time.time()
        except Exception:
            pass


class TradingGUI:
    """Main GUI application for the trading simulator"""

    # Color scheme
    BG_DARK = '#1e1e1e'
    BG_MEDIUM = '#2d2d2d'
    BG_LIGHT = '#3d3d3d'
    FG_GREEN = '#00ff00'
    FG_RED = '#ff0000'
    FG_YELLOW = '#ffff00'
    FG_WHITE = '#ffffff'
    FG_GRAY = '#cccccc'

    def __init__(self, root: tk.Tk):
        self.root = root
        self.client: Optional[FIXClient] = None
        self.selected_instrument = Instrument.EURUSD
        self.data_queue = Queue()

        self._setup_window()
        self._setup_ui()

    def _setup_window(self) -> None:
        """Configure the main window"""
        self.root.title("FIX Trading Simulator - Live Dashboard")
        self.root.geometry("1400x850")
        self.root.configure(bg=self.BG_DARK)
        self.root.protocol("WM_DELETE_WINDOW", self._on_closing)

    def _setup_ui(self) -> None:
        """Setup all UI components"""
        self._create_control_panel()
        self._create_main_layout()

    def _create_control_panel(self) -> None:
        """Create the top control panel"""
        control_frame = tk.Frame(self.root, bg=self.BG_MEDIUM, height=60)
        control_frame.pack(fill=tk.X, padx=5, pady=5)
        control_frame.pack_propagate(False)

        # Title
        tk.Label(
            control_frame,
            text="FIX Trading Simulator",
            font=('Arial', 16, 'bold'),
            bg=self.BG_MEDIUM,
            fg=self.FG_GREEN
        ).pack(side=tk.LEFT, padx=10)

        # Start button
        self.start_btn = tk.Button(
            control_frame,
            text="▶ START",
            font=('Arial', 12, 'bold'),
            bg='#00aa00',
            fg='white',
            command=self.start_trading,
            width=10
        )
        self.start_btn.pack(side=tk.LEFT, padx=5)

        # Stop button
        self.stop_btn = tk.Button(
            control_frame,
            text="⏹ STOP",
            font=('Arial', 12, 'bold'),
            bg='#aa0000',
            fg='white',
            command=self.stop_trading,
            width=10,
            state=tk.DISABLED
        )
        self.stop_btn.pack(side=tk.LEFT, padx=5)

        # Status label
        self.status_label = tk.Label(
            control_frame,
            text="⚫ Disconnected",
            font=('Arial', 11),
            bg=self.BG_MEDIUM,
            fg=self.FG_RED
        )
        self.status_label.pack(side=tk.RIGHT, padx=10)

    def _create_main_layout(self) -> None:
        """Create the main application layout"""
        main_frame = tk.Frame(self.root, bg=self.BG_DARK)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Left panel
        left_panel = tk.Frame(main_frame, bg=self.BG_MEDIUM, width=380)
        left_panel.pack(side=tk.LEFT, fill=tk.BOTH, padx=(0, 5))
        left_panel.pack_propagate(False)

        self._create_stats_panel(left_panel)
        self._create_instrument_selector(left_panel)
        self._create_order_book_panel(left_panel)

        # Right panel
        right_panel = tk.Frame(main_frame, bg=self.BG_MEDIUM)
        right_panel.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)

        self._create_chart_panel(right_panel)
        self._create_log_panel(right_panel)

    def _create_stats_panel(self, parent: tk.Frame) -> None:
        """Create statistics display panel"""
        stats_frame = tk.LabelFrame(
            parent,
            text="STATISTICS",
            font=('Arial', 11, 'bold'),
            bg=self.BG_MEDIUM,
            fg=self.FG_GREEN,
            bd=2
        )
        stats_frame.pack(fill=tk.X, padx=5, pady=5)

        self.stats_text = tk.Text(
            stats_frame,
            height=8,
            bg=self.BG_DARK,
            fg=self.FG_GREEN,
            font=('Consolas', 10),
            relief=tk.FLAT
        )
        self.stats_text.pack(fill=tk.BOTH, padx=5, pady=5)

    def _create_instrument_selector(self, parent: tk.Frame) -> None:
        """Create instrument selection panel"""
        selector_frame = tk.LabelFrame(
            parent,
            text="SELECT INSTRUMENT",
            font=('Arial', 11, 'bold'),
            bg=self.BG_MEDIUM,
            fg=self.FG_GREEN,
            bd=2
        )
        selector_frame.pack(fill=tk.X, padx=5, pady=5)

        for inst in Instrument:
            if inst != Instrument.COUNT:
                btn = tk.Button(
                    selector_frame,
                    text=inst.name,
                    font=('Arial', 10, 'bold'),
                    bg=self.BG_LIGHT,
                    fg=self.FG_WHITE,
                    command=lambda i=inst: self.select_instrument(i)
                )
                btn.pack(fill=tk.X, padx=5, pady=2)

    def _create_order_book_panel(self, parent: tk.Frame) -> None:
        """Create order book display panel"""
        book_frame = tk.LabelFrame(
            parent,
            text="ORDER BOOK",
            font=('Arial', 11, 'bold'),
            bg=self.BG_MEDIUM,
            fg=self.FG_GREEN,
            bd=2
        )
        book_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Create a frame for the table
        table_container = tk.Frame(book_frame, bg=self.BG_DARK)
        table_container.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)

        # Scrollbar
        scrollbar = ttk.Scrollbar(table_container)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        # Create Treeview for table display
        self.book_tree = ttk.Treeview(
            table_container,
            columns=('bid_qty', 'bid_price', 'ask_price', 'ask_qty'),
            show='headings',
            height=15,
            yscrollcommand=scrollbar.set
        )
        scrollbar.config(command=self.book_tree.yview)

        # Configure columns
        self.book_tree.heading('bid_qty', text='Bid Qty')
        self.book_tree.heading('bid_price', text='Bid Price')
        self.book_tree.heading('ask_price', text='Ask Price')
        self.book_tree.heading('ask_qty', text='Ask Qty')

        self.book_tree.column('bid_qty', width=80, anchor='e')
        self.book_tree.column('bid_price', width=90, anchor='e')
        self.book_tree.column('ask_price', width=90, anchor='w')
        self.book_tree.column('ask_qty', width=80, anchor='w')

        # Style the Treeview
        style = ttk.Style()
        style.theme_use('default')
        style.configure(
            'Treeview',
            background=self.BG_DARK,
            foreground=self.FG_WHITE,
            fieldbackground=self.BG_DARK,
            borderwidth=0
        )
        style.configure('Treeview.Heading', background=self.BG_MEDIUM, foreground=self.FG_GREEN, font=('Arial', 9, 'bold'))
        style.map('Treeview', background=[('selected', self.BG_LIGHT)])

        self.book_tree.pack(fill=tk.BOTH, expand=True)

    def _create_chart_panel(self, parent: tk.Frame) -> None:
        """Create chart display panel"""
        chart_frame = tk.Frame(parent, bg=self.BG_MEDIUM)
        chart_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 5))

        self.fig = Figure(figsize=(10, 6), facecolor=self.BG_DARK)
        self.ax_ohlc = self.fig.add_subplot(211, facecolor=self.BG_DARK)
        self.ax_volume = self.fig.add_subplot(212, facecolor=self.BG_DARK, sharex=self.ax_ohlc)
        self.fig.tight_layout()

        self.canvas = FigureCanvasTkAgg(self.fig, master=chart_frame)
        self.canvas.draw()
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

    def _create_log_panel(self, parent: tk.Frame) -> None:
        """Create log display panel"""
        log_frame = tk.LabelFrame(
            parent,
            text="FIX LOGS",
            font=('Arial', 11, 'bold'),
            bg=self.BG_MEDIUM,
            fg=self.FG_GREEN,
            bd=2
        )
        log_frame.pack(fill=tk.X, pady=(5, 0))

        self.log_text = tk.Text(
            log_frame,
            height=10,
            bg=self.BG_DARK,
            fg=self.FG_GRAY,
            font=('Consolas', 9),
            relief=tk.FLAT
        )
        self.log_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=5, pady=5)

        log_scrollbar = ttk.Scrollbar(log_frame, command=self.log_text.yview)
        log_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.log_text.config(yscrollcommand=log_scrollbar.set)
        self.log_text.configure(state="disabled")

    def select_instrument(self, instrument: Instrument) -> None:
        """Handle instrument selection"""
        self.selected_instrument = instrument
        if self.client and self.client.running:
            self.client.send_market_data_request(instrument)

    def start_trading(self) -> None:
        """Start the trading client"""
        print("[GUI] START button clicked")

        self.start_btn.config(state=tk.DISABLED)
        self.stop_btn.config(state=tk.NORMAL)
        self.status_label.config(text="🟡 Connecting...", fg=self.FG_YELLOW)

        # Clear logs
        self.log_text.configure(state="normal")
        self.log_text.delete('1.0', tk.END)
        self.log_text.configure(state="disabled")

        # Redirect stdout/stderr
        sys.stdout = TextRedirector(self.log_text, "stdout")
        sys.stderr = TextRedirector(self.log_text, "stderr")

        print("[GUI] Starting FIX client...")

        # Start client
        self.client = FIXClient(gui=self, data_queue=self.data_queue)
        threading.Thread(target=self.client.run, daemon=True).start()

        print("[GUI] FIX client thread started, starting GUI update loop...")

        # Start GUI update loop - THIS IS CRITICAL
        self.root.after(500, self.update_gui)  # Start after 500ms to let connection establish

        print("[GUI] GUI update loop scheduled")

    def stop_trading(self) -> None:
        """Stop the trading client"""
        if self.client:
            self.client.stop()

        self.start_btn.config(state=tk.NORMAL)
        self.stop_btn.config(state=tk.DISABLED)
        self.status_label.config(text="⚫ Disconnected", fg=self.FG_RED)

        # Restore stdout/stderr
        sys.stdout = sys.__stdout__
        sys.stderr = sys.__stderr__

    def process_queue(self) -> None:
        """Process incoming data from the queue"""
        # Process maximum 50 items per update to prevent lag
        processed = 0
        max_per_update = 50

        try:
            while processed < max_per_update:
                data = self.data_queue.get_nowait()

                if isinstance(data, SnapshotData):
                    self.client.order_book.apply_snapshot(data.symbol, data.entries)
                    # print(f"[GUI] Applied snapshot for {data.symbol.name}")

                elif isinstance(data, IncrementalData):
                    # Check if it's a trade (type 2)
                    if data.entry.md_entry_type == 2:
                        self.client.order_book.update_trade(
                            data.symbol,
                            data.entry.md_entry_px,
                            data.entry.md_entry_size
                        )
                        # print(f"[GUI] Recorded trade for {data.symbol.name}: {data.entry.md_entry_px:.5f} x {data.entry.md_entry_size}")
                    else:
                        # Regular bid/ask update
                        self.client.order_book.apply_incremental(data.symbol, data.entry)

                processed += 1

        except Empty:
            pass

        # if processed > 0:
        #     print(f"[GUI] Processed {processed} queue items")

    def update_gui(self) -> None:
        """Main GUI update loop"""
        if not self.client or not self.client.running:
            # print("[GUI] Update stopped - client not running")
            return

        # print("[GUI] === Update cycle starting ===")
        self.process_queue()
        self.update_stats()
        self.update_order_book()
        self.update_charts()
        # print("[GUI] === Update cycle complete ===")

        # Update less frequently for better performance (250ms instead of 100ms)
        self.root.after(250, self.update_gui)

    def update_stats(self) -> None:
        """Update statistics display"""
        if not self.client:
            return

        elapsed = 0
        if self.client.start_time:
            elapsed = (datetime.now() - self.client.start_time).total_seconds()

        rate = self.client.orders_sent / elapsed if elapsed > 0 else 0
        fill_rate = 0
        if self.client.orders_sent > 0:
            fill_rate = (self.client.fills_received / self.client.orders_sent * 100)

        stats = f"""Runtime:            {elapsed:.1f}s
Orders Sent:        {self.client.orders_sent}
Executions Rcvd:    {self.client.executions_received}
Fills Rcvd:         {self.client.fills_received}
Total Filled Qty:   {self.client.total_filled_qty}
Order Rate:         {rate:.2f} orders/sec
Fill Rate:          {fill_rate:.1f}%"""

        self.stats_text.delete('1.0', tk.END)
        self.stats_text.insert('1.0', stats)

        # print(f"[GUI] Updated stats - Orders: {self.client.orders_sent}, Fills: {self.client.fills_received}")

    def update_order_book(self) -> None:
        """Update order book display"""
        if not self.client:
            return

        bids, asks = self.client.order_book.get_top_of_book(
            self.selected_instrument,
            depth=15
        )

        # print(f"[GUI] Order book for {self.selected_instrument.name}: {len(bids)} bids, {len(asks)} asks")

        # Clear existing items
        for item in self.book_tree.get_children():
            self.book_tree.delete(item)

        # Populate table (bids and asks side by side)
        max_rows = max(len(bids), len(asks))

        for i in range(max_rows):
            bid_qty = f"{bids[i][1]:,}" if i < len(bids) else ""
            bid_price = f"{bids[i][0]:.5f}" if i < len(bids) else ""
            ask_price = f"{asks[i][0]:.5f}" if i < len(asks) else ""
            ask_qty = f"{asks[i][1]:,}" if i < len(asks) else ""

            self.book_tree.insert('', 'end', values=(bid_qty, bid_price, ask_price, ask_qty))

    def update_charts(self) -> None:
        """Update OHLC and volume charts"""
        if not self.client:
            print("[CHART] No client")
            return

        ohlc = self.client.order_book.ohlc_data[self.selected_instrument]

        print(f"[CHART] {self.selected_instrument.name} has {len(ohlc['timestamps'])} candles")

        if len(ohlc['timestamps']) < 1:
            print("[CHART] Not enough candle data yet")
            return

        try:
            # Clear axes
            self.ax_ohlc.clear()
            self.ax_volume.clear()

            # Limit data points for performance (last 50 candles)
            max_candles = 50
            start_idx = max(0, len(ohlc['timestamps']) - max_candles)

            print(f"[CHART] Drawing {len(ohlc['timestamps']) - start_idx} candles starting from index {start_idx}")

            # Draw candlesticks
            for i in range(start_idx, len(ohlc['timestamps'])):
                x = i - start_idx
                o = ohlc['open'][i]
                h = ohlc['high'][i]
                l = ohlc['low'][i]
                c = ohlc['close'][i]

                color = self.FG_GREEN if c >= o else self.FG_RED

                # Draw wick
                self.ax_ohlc.plot([x, x], [l, h], color=color, linewidth=1)

                # Draw body
                body_bottom = min(o, c)
                body_height = abs(c - o) if abs(c - o) > 0 else 0.00001
                rect = mpatches.Rectangle(
                    (x - 0.3, body_bottom),
                    0.6,
                    body_height,
                    facecolor=color,
                    edgecolor=color
                )
                self.ax_ohlc.add_patch(rect)

            # Style OHLC chart
            self.ax_ohlc.set_title(
                f'{self.selected_instrument.name} OHLC',
                color=self.FG_GREEN,
                fontsize=14,
                fontweight='bold'
            )
            self.ax_ohlc.tick_params(axis='y', colors=self.FG_GREEN)
            self.ax_ohlc.tick_params(axis='x', colors=self.FG_GREEN)
            self.ax_ohlc.relim()
            self.ax_ohlc.autoscale_view()

            # Draw volume bars
            volumes = ohlc.get('volume', [])[start_idx:]
            if volumes:
                colors = [
                    self.FG_GREEN if ohlc['close'][i] >= ohlc['open'][i] else self.FG_RED
                    for i in range(start_idx, len(ohlc['close']))
                ]
                self.ax_volume.bar(range(len(volumes)), volumes, width=0.8, color=colors)
                self.ax_volume.set_title('Volume', color=self.FG_GREEN, fontsize=12)
                self.ax_volume.tick_params(colors=self.FG_GREEN)
                self.ax_volume.relim()
                self.ax_volume.autoscale_view()

            self.fig.tight_layout()

            # Only redraw canvas if it's been at least 0.3 seconds since last redraw
            if not hasattr(self, '_last_chart_update'):
                self._last_chart_update = 0

            if time.time() - self._last_chart_update > 0.3:
                self.canvas.draw()
                self._last_chart_update = time.time()
                print(f"[CHART] Canvas redrawn successfully")

        except Exception as e:
            print(f"[CHART] Error updating charts: {e}")
            traceback.print_exc()

    def _on_closing(self) -> None:
        """Handle window closing"""
        if self.client:
            self.client.stop()
        self.root.destroy()


# ============================================================================
# MAIN ENTRY POINT
# ============================================================================

def main():
    """Main application entry point"""
    root = tk.Tk()
    app = TradingGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()