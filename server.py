import socket
import threading
import mss
from PIL import Image
import time
import struct
import logging
from datetime import datetime
import hashlib
import json
import subprocess

# 配置日志
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

class ScreenStreamServer:
    def __init__(self, host='0.0.0.0', port=20500, password=None):
        self.host = host
        self.port = port
        self.password = password  # 可选密码认证
        self.server_socket = None
        self.clients = []
        self.client_info = {}  # 存储客户端信息
        self.running = False
        self.sct = mss.mss()
        
        # 显示配置 (128x64 OLED) - 修改为128x64
        self.width = 128
        self.height = 64
        self.pages = self.height // 8  # 8页
        
        # 性能统计
        self.frame_count = 0
        self.last_stats_time = time.time()
        
    def start(self):
        """启动服务器"""
        try:
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.server_socket.bind((self.host, self.port))
            self.server_socket.listen(10)
            self.running = True
            
            logger.info(f"服务器启动成功")
            logger.info(f"监听地址: {self.host}:{self.port}")
            logger.info(f"屏幕尺寸: {self.width}x{self.height}")
            
            # 获取本机IP地址
            hostname = socket.gethostname()
            local_ip = socket.gethostbyname(hostname)
            logger.info(f"本机IP地址: {local_ip}")
            logger.info(f"公网IP需要在路由器中设置端口转发或使用NAT")
            
            # 启动接受客户端线程
            accept_thread = threading.Thread(target=self.accept_clients, daemon=True)
            accept_thread.start()
            
            # 启动屏幕捕获和广播线程
            broadcast_thread = threading.Thread(target=self.broadcast_screen, daemon=True)
            broadcast_thread.start()
            
            # 启动统计线程
            stats_thread = threading.Thread(target=self.show_statistics, daemon=True)
            stats_thread.start()
            
            # 主线程保持运行
            while self.running:
                time.sleep(1)
                
        except Exception as e:
            logger.error(f"服务器启动失败: {e}")
            self.stop()
    
    def authenticate_client(self, client_socket):
        """客户端认证（可选）"""
        if not self.password:
            return True
        
        try:
            # 发送认证请求
            auth_request = json.dumps({"type": "auth", "required": True})
            client_socket.send(auth_request.encode() + b'\n')
            
            # 接收认证响应
            response = client_socket.recv(1024).decode()
            auth_data = json.loads(response)
            
            if auth_data.get('password') == self.password:
                client_socket.send(b'{"status": "ok"}\n')
                return True
            else:
                client_socket.send(b'{"status": "failed"}\n')
                return False
        except:
            return False
    
    def accept_clients(self):
        """接受客户端连接"""
        while self.running:
            try:
                self.server_socket.settimeout(1.0)
                client_socket, addr = self.server_socket.accept()
                
                logger.info(f"新客户端连接尝试: {addr}")
                
                # 认证客户端
                if self.authenticate_client(client_socket):
                    self.clients.append(client_socket)
                    self.client_info[client_socket] = {
                        'addr': addr,
                        'connect_time': datetime.now(),
                        'frames_received': 0
                    }
                    logger.info(f"客户端认证成功: {addr}，当前连接数: {len(self.clients)}")
                else:
                    client_socket.close()
                    logger.warning(f"客户端认证失败: {addr}")
                    
            except socket.timeout:
                continue
            except Exception as e:
                if self.running:
                    logger.error(f"接受连接错误: {e}")
    
    def capture_screen(self):
        """捕获屏幕并处理"""
        try:
            # 获取主显示器
            monitor = self.sct.monitors[1]
            screenshot = self.sct.grab(monitor)
            
            # 转换为PIL图像
            img = Image.frombytes("RGB", screenshot.size, screenshot.bgra, "raw", "BGRX")
            
            # 调整到OLED分辨率 (128x64)
            img = img.resize((self.width, self.height), Image.Resampling.LANCZOS)
            
            # 转换为灰度图
            img_gray = img.convert('L')
            
            # 自适应二值化（效果更好）
            from PIL import ImageOps
            img_binary = ImageOps.autocontrast(img_gray)
            threshold = 128
            img_binary = img_binary.point(lambda x: 255 if x > threshold else 0, '1')
            
            return img_binary
            
        except Exception as e:
            logger.error(f"屏幕捕获失败: {e}")
            # 返回空白图像
            return Image.new('1', (self.width, self.height), 0)
    
    def image_to_bytes(self, img):
        """转换为OLED需要的字节格式 (128x64: 8页)"""
        img_bytes = bytearray()
        pixels = list(img.getdata())
        
        # 按页组织 (128x64: 8页)
        for page in range(self.pages):
            for col in range(self.width):
                byte = 0
                # 每个字节代表一列中连续的8个像素
                for row_bit in range(8):
                    pixel_y = page * 8 + row_bit
                    pixel_index = pixel_y * self.width + col
                    if pixel_index < len(pixels) and pixels[pixel_index] == 255:
                        byte |= (1 << row_bit)  # bit0是最上方像素
                img_bytes.append(byte)
        
        return img_bytes
    
    def send_to_client(self, client_socket, img_bytes):
        """发送数据到单个客户端"""
        try:
            # 发送数据长度（2字节，大端序）
            client_socket.send(struct.pack('>H', len(img_bytes)))
            # 发送图像数据
            client_socket.send(img_bytes)
            return True
        except:
            return False
    
    def broadcast_screen(self, fps=15):
        """广播屏幕内容到所有客户端"""
        interval = 1.0 / fps
        last_frame_time = time.time()
        
        while self.running:
            try:
                start_time = time.time()
                
                # 捕获屏幕
                img = self.capture_screen()
                img_bytes = self.image_to_bytes(img)
                
                # 发送到所有客户端
                dead_clients = []
                for client in self.clients:
                    if not self.send_to_client(client, img_bytes):
                        dead_clients.append(client)
                
                # 移除断开的客户端
                for dead in dead_clients:
                    if dead in self.clients:
                        self.clients.remove(dead)
                        if dead in self.client_info:
                            info = self.client_info[dead]
                            logger.info(f"客户端断开: {info['addr']}")
                            del self.client_info[dead]
                        dead.close()
                
                # 更新统计
                self.frame_count += 1
                
                # 帧率控制
                elapsed = time.time() - start_time
                if elapsed < interval:
                    time.sleep(interval - elapsed)
                    
            except Exception as e:
                logger.error(f"广播错误: {e}")
                time.sleep(1)
    
    def show_statistics(self):
        """显示统计信息"""
        while self.running:
            time.sleep(10)  # 每10秒显示一次
            if self.frame_count > 0:
                elapsed = time.time() - self.last_stats_time
                fps = self.frame_count / elapsed if elapsed > 0 else 0
                logger.info(f"统计 - FPS: {fps:.1f}, 客户端数: {len(self.clients)}")
                self.frame_count = 0
                self.last_stats_time = time.time()
    
    def stop(self):
        """停止服务器"""
        logger.info("正在关闭服务器...")
        self.running = False
        
        # 关闭所有客户端连接
        for client in self.clients:
            try:
                client.close()
            except:
                pass
        
        # 关闭服务器socket
        if self.server_socket:
            self.server_socket.close()
        
        logger.info("服务器已关闭")

def main():
    # 配置参数
    HOST = '0.0.0.0'  # 监听所有网卡
    PORT = 20500
    PASSWORD = None  # 设置密码可启用认证，如 "mypassword"
    
    # 可选：从配置文件读取
    # import configparser
    # config = configparser.ConfigParser()
    # config.read('server.ini')
    # HOST = config.get('server', 'host', fallback='0.0.0.0')
    # PORT = config.getint('server', 'port', fallback=19500)
    # PASSWORD = config.get('server', 'password', fallback=None)
    
    server = ScreenStreamServer(HOST, PORT, PASSWORD)
    
    try:
        server.start()
    except KeyboardInterrupt:
        logger.info("\n收到停止信号")
        server.stop()
    except Exception as e:
        logger.error(f"服务器异常: {e}")
        server.stop()

if __name__ == "__main__":
    process = subprocess.Popen(["frpc", "-c", "frpc.toml"],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)#打开内网穿透
    main()