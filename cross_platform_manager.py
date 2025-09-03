# ZeroTier Cross-Platform Management Suite
# Aplicação multiplataforma para gestão do ZeroTier

import tkinter as tk
from tkinter import ttk, messagebox, simpledialog
import requests
import json
import threading
import time
import platform
import os
import sys
from datetime import datetime, timedelta

class ZeroTierCrossPlatformManager:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("ZeroTier Management Console - Cross Platform")
        self.root.geometry("1100x750")
        
        # Detectar plataforma
        self.platform = platform.system().lower()
        self.setup_platform_specific()
        
        # Configuração da API
        self.api_base = "http://localhost:9993"
        self.auth_token = self.get_auth_token()
        
        # Dados
        self.status_data = {}
        self.networks_data = []
        self.peers_data = []
        
        self.setup_ui()
        self.start_auto_refresh()
        
    def setup_platform_specific(self):
        """Configurações específicas da plataforma"""
        if self.platform == "windows":
            self.root.configure(bg='#f0f0f0')
            self.icon_prefix = "🪟"
        elif self.platform == "darwin":  # macOS
            self.root.configure(bg='#f5f5f7')
            self.icon_prefix = "🍎"
        elif self.platform == "linux":
            self.root.configure(bg='#ffffff')
            self.icon_prefix = "🐧"
        else:
            self.root.configure(bg='#f8f9fa')
            self.icon_prefix = "💻"
    
    def get_auth_token(self):
        """Obtém o token de autenticação baseado na plataforma"""
        token_paths = []
        
        if self.platform == "windows":
            # Windows paths
            token_paths = [
                os.path.expanduser("~/AppData/Local/ZeroTier/One/authtoken.secret"),
                "C:/ProgramData/ZeroTier/One/authtoken.secret",
                os.path.expanduser("~/ZeroTier/authtoken.secret")
            ]
        elif self.platform == "darwin":  # macOS
            # macOS paths
            token_paths = [
                "/Library/Application Support/ZeroTier/One/authtoken.secret",
                os.path.expanduser("~/Library/Application Support/ZeroTier/One/authtoken.secret"),
                "/var/lib/zerotier-one/authtoken.secret"
            ]
        elif self.platform == "linux":
            # Linux paths
            token_paths = [
                "/var/lib/zerotier-one/authtoken.secret",
                os.path.expanduser("~/.zerotier/authtoken.secret"),
                "/etc/zerotier/authtoken.secret",
                "/usr/local/var/lib/zerotier-one/authtoken.secret"
            ]
        else:
            # FreeBSD e outros Unix-like
            token_paths = [
                "/var/db/zerotier-one/authtoken.secret",
                "/usr/local/var/lib/zerotier-one/authtoken.secret",
                "/var/lib/zerotier-one/authtoken.secret"
            ]
        
        for token_path in token_paths:
            try:
                if os.path.exists(token_path):
                    with open(token_path, 'r') as f:
                        return f.read().strip()
            except Exception as e:
                continue
        
        # Se não encontrou, tentar obter via variável de ambiente
        token = os.environ.get('ZEROTIER_AUTH_TOKEN')
        if token:
            return token
            
        messagebox.showerror("Erro", f"Token de autenticação não encontrado para {self.platform}")
        return None
    
    def get_zerotier_service_info(self):
        """Obtém informações específicas do serviço por plataforma"""
        if self.platform == "windows":
            return {
                "service_name": "ZeroTier One",
                "config_path": "C:/ProgramData/ZeroTier/One/local.conf",
                "data_path": "C:/ProgramData/ZeroTier/One/"
            }
        elif self.platform == "darwin":
            return {
                "service_name": "com.zerotier.one",
                "config_path": "/Library/Application Support/ZeroTier/One/local.conf",
                "data_path": "/Library/Application Support/ZeroTier/One/"
            }
        elif self.platform == "linux":
            return {
                "service_name": "zerotier-one",
                "config_path": "/var/lib/zerotier-one/local.conf",
                "data_path": "/var/lib/zerotier-one/"
            }
        else:
            return {
                "service_name": "zerotier-one",
                "config_path": "/var/db/zerotier-one/local.conf",
                "data_path": "/var/db/zerotier-one/"
            }
    
    def api_request(self, endpoint, method='GET', data=None):
        """Faz requisição para a API do ZeroTier"""
        try:
            headers = {
                'X-ZT1-Auth': self.auth_token,
                'Content-Type': 'application/json'
            }
            
            url = f"{self.api_base}{endpoint}"
            
            if method == 'GET':
                response = requests.get(url, headers=headers, timeout=5)
            elif method == 'POST':
                response = requests.post(url, headers=headers, json=data, timeout=5)
            elif method == 'DELETE':
                response = requests.delete(url, headers=headers, timeout=5)
            
            if response.status_code == 200:
                return response.json()
            else:
                raise Exception(f"HTTP {response.status_code}: {response.text}")
                
        except Exception as e:
            print(f"Erro na API: {e}")
            return None
    
    def setup_ui(self):
        """Configura a interface do usuário"""
        # Estilo baseado na plataforma
        style = ttk.Style()
        
        if self.platform == "darwin":
            style.theme_use('aqua')
        elif self.platform == "windows":
            style.theme_use('vista')
        else:
            style.theme_use('clam')
        
        # Frame principal
        main_frame = ttk.Frame(self.root, padding="15")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # Configurar grid
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(1, weight=1)
        main_frame.rowconfigure(3, weight=1)
        
        # Título com ícone da plataforma
        platform_name = {
            "windows": "Windows",
            "darwin": "macOS", 
            "linux": "Linux",
            "freebsd": "FreeBSD"
        }.get(self.platform, "Unix")
        
        title_text = f"{self.icon_prefix} ZeroTier Management Console - {platform_name}"
        title_label = tk.Label(main_frame, text=title_text, 
                              font=('Arial', 16, 'bold'), 
                              bg=self.root.cget('bg'))
        title_label.grid(row=0, column=0, columnspan=3, pady=(0, 20))
        
        # Informações da plataforma
        platform_info = self.get_zerotier_service_info()
        platform_frame = ttk.LabelFrame(main_frame, text="Informações da Plataforma", padding="10")
        platform_frame.grid(row=1, column=0, columnspan=3, sticky=(tk.W, tk.E), pady=(0, 15))
        
        tk.Label(platform_frame, text=f"Sistema: {platform.system()} {platform.release()}", 
                font=('Arial', 9)).grid(row=0, column=0, sticky=tk.W)
        tk.Label(platform_frame, text=f"Arquitetura: {platform.machine()}", 
                font=('Arial', 9)).grid(row=0, column=1, sticky=tk.W, padx=(20, 0))
        tk.Label(platform_frame, text=f"Serviço: {platform_info['service_name']}", 
                font=('Arial', 9)).grid(row=1, column=0, sticky=tk.W)
        tk.Label(platform_frame, text=f"Config: {platform_info['config_path']}", 
                font=('Arial', 9)).grid(row=1, column=1, sticky=tk.W, padx=(20, 0))
        
        # Frame de status (esquerda)
        status_frame = ttk.LabelFrame(main_frame, text="Status do Serviço", padding="10")
        status_frame.grid(row=2, column=0, sticky=(tk.W, tk.E, tk.N), padx=(0, 10))
        
        # Labels de status
        self.status_labels = {}
        status_fields = [
            ("Estado:", "status"),
            ("Versão:", "version"),
            ("Node ID:", "node_id"),
            ("Endereço:", "address"),
            ("Porta:", "port"),
            ("Uptime:", "uptime"),
            ("Online:", "online")
        ]
        
        for i, (label, key) in enumerate(status_fields):
            tk.Label(status_frame, text=label, font=('Arial', 9, 'bold')).grid(
                row=i, column=0, sticky=tk.W, pady=2)
            self.status_labels[key] = tk.Label(status_frame, text="-", font=('Arial', 9))
            self.status_labels[key].grid(row=i, column=1, sticky=tk.W, padx=(10, 0), pady=2)
        
        # Frame de estatísticas (centro)
        stats_frame = ttk.LabelFrame(main_frame, text="Estatísticas", padding="10")
        stats_frame.grid(row=2, column=1, sticky=(tk.W, tk.E, tk.N), padx=(5, 5))
        
        # Labels de estatísticas
        self.stats_labels = {}
        stats_fields = [
            ("Redes Ativas:", "networks"),
            ("Peers Conectados:", "peers"),
            ("Última Atualização:", "last_update"),
            ("Planeta ID:", "planet_id"),
            ("Controlador:", "controller"),
            ("TCP Fallback:", "tcp_fallback"),
            ("Interface Prefix:", "if_prefix")
        ]
        
        for i, (label, key) in enumerate(stats_fields):
            tk.Label(stats_frame, text=label, font=('Arial', 9, 'bold')).grid(
                row=i, column=0, sticky=tk.W, pady=2)
            self.stats_labels[key] = tk.Label(stats_frame, text="-", font=('Arial', 9))
            self.stats_labels[key].grid(row=i, column=1, sticky=tk.W, padx=(10, 0), pady=2)
        
        # Frame de ações (direita)
        actions_frame = ttk.LabelFrame(main_frame, text="Ações do Sistema", padding="10")
        actions_frame.grid(row=2, column=2, sticky=(tk.W, tk.E, tk.N), padx=(10, 0))
        
        # Botões de ação específicos da plataforma
        ttk.Button(actions_frame, text="🔄 Atualizar", command=self.refresh_data).grid(
            row=0, column=0, sticky=(tk.W, tk.E), pady=2)
        ttk.Button(actions_frame, text="📊 Status Serviço", command=self.check_service_status).grid(
            row=1, column=0, sticky=(tk.W, tk.E), pady=2)
        ttk.Button(actions_frame, text="📁 Abrir Config", command=self.open_config_folder).grid(
            row=2, column=0, sticky=(tk.W, tk.E), pady=2)
        ttk.Button(actions_frame, text="📋 Info Sistema", command=self.show_system_info).grid(
            row=3, column=0, sticky=(tk.W, tk.E), pady=2)
        ttk.Button(actions_frame, text="🔧 Ferramentas", command=self.open_tools_menu).grid(
            row=4, column=0, sticky=(tk.W, tk.E), pady=2)
        
        # Frame de redes
        networks_frame = ttk.LabelFrame(main_frame, text="Redes Conectadas", padding="10")
        networks_frame.grid(row=3, column=0, columnspan=3, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(20, 0))
        networks_frame.columnconfigure(0, weight=1)
        networks_frame.rowconfigure(0, weight=1)
        
        # Treeview para redes
        columns = ('ID', 'Nome', 'Status', 'IP', 'Tipo', 'Peers')
        self.networks_tree = ttk.Treeview(networks_frame, columns=columns, show='headings', height=8)
        
        # Configurar colunas
        column_widths = {'ID': 140, 'Nome': 120, 'Status': 80, 'IP': 140, 'Tipo': 80, 'Peers': 60}
        for col in columns:
            self.networks_tree.heading(col, text=col)
            self.networks_tree.column(col, width=column_widths.get(col, 100))
        
        # Scrollbars
        v_scrollbar = ttk.Scrollbar(networks_frame, orient=tk.VERTICAL, command=self.networks_tree.yview)
        h_scrollbar = ttk.Scrollbar(networks_frame, orient=tk.HORIZONTAL, command=self.networks_tree.xview)
        self.networks_tree.configure(yscrollcommand=v_scrollbar.set, xscrollcommand=h_scrollbar.set)
        
        self.networks_tree.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        v_scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S))
        h_scrollbar.grid(row=1, column=0, sticky=(tk.W, tk.E))
        
        # Frame de botões para redes
        network_buttons_frame = ttk.Frame(networks_frame)
        network_buttons_frame.grid(row=2, column=0, columnspan=2, pady=(10, 0))
        
        ttk.Button(network_buttons_frame, text="➕ Entrar em Rede", command=self.join_network_dialog).pack(side=tk.LEFT, padx=(0, 10))
        ttk.Button(network_buttons_frame, text="❌ Sair da Rede", command=self.leave_network).pack(side=tk.LEFT, padx=(0, 10))
        ttk.Button(network_buttons_frame, text="ℹ️ Detalhes", command=self.show_network_details).pack(side=tk.LEFT, padx=(0, 10))
        ttk.Button(network_buttons_frame, text="📊 Peers", command=self.show_peers).pack(side=tk.LEFT)
        
        # Carregar dados iniciais
        self.refresh_data()
    
    def check_service_status(self):
        """Verifica status do serviço ZeroTier específico da plataforma"""
        def _check():
            platform_info = self.get_zerotier_service_info()
            service_name = platform_info['service_name']
            
            try:
                if self.platform == "windows":
                    import subprocess
                    result = subprocess.run(['sc', 'query', service_name], 
                                          capture_output=True, text=True)
                    status = "Rodando" if "RUNNING" in result.stdout else "Parado"
                elif self.platform == "darwin":
                    import subprocess
                    result = subprocess.run(['launchctl', 'list', service_name], 
                                          capture_output=True, text=True)
                    status = "Rodando" if result.returncode == 0 else "Parado"
                else:  # Linux/FreeBSD
                    import subprocess
                    result = subprocess.run(['systemctl', 'is-active', service_name], 
                                          capture_output=True, text=True)
                    status = "Rodando" if "active" in result.stdout else "Parado"
                
                self.root.after(0, lambda: messagebox.showinfo("Status do Serviço", 
                    f"Serviço {service_name}: {status}"))
                    
            except Exception as e:
                self.root.after(0, lambda: messagebox.showerror("Erro", 
                    f"Erro ao verificar serviço: {e}"))
        
        threading.Thread(target=_check, daemon=True).start()
    
    def open_config_folder(self):
        """Abre a pasta de configuração do ZeroTier"""
        platform_info = self.get_zerotier_service_info()
        config_dir = os.path.dirname(platform_info['config_path'])
        
        try:
            if self.platform == "windows":
                os.startfile(config_dir)
            elif self.platform == "darwin":
                os.system(f'open "{config_dir}"')
            else:  # Linux/FreeBSD
                os.system(f'xdg-open "{config_dir}" 2>/dev/null || nautilus "{config_dir}" 2>/dev/null || dolphin "{config_dir}" 2>/dev/null')
        except Exception as e:
            messagebox.showerror("Erro", f"Erro ao abrir pasta: {e}")
    
    def show_system_info(self):
        """Mostra informações detalhadas do sistema"""
        import psutil
        
        try:
            cpu_percent = psutil.cpu_percent(interval=1)
            memory = psutil.virtual_memory()
            disk = psutil.disk_usage('/')
            
            info = f"""
Informações do Sistema:

Sistema Operacional: {platform.system()} {platform.release()}
Arquitetura: {platform.machine()}
Processador: {platform.processor()}
Python: {platform.python_version()}

Recursos:
CPU: {cpu_percent}%
Memória: {memory.percent}% ({memory.used // (1024**3)} GB / {memory.total // (1024**3)} GB)
Disco: {disk.percent}% ({disk.used // (1024**3)} GB / {disk.total // (1024**3)} GB)

ZeroTier:
API Base: {self.api_base}
Token: {'Configurado' if self.auth_token else 'Não encontrado'}
            """
            
            messagebox.showinfo("Informações do Sistema", info.strip())
            
        except ImportError:
            messagebox.showinfo("Informações do Sistema", 
                f"Sistema: {platform.system()} {platform.release()}\n"
                f"Arquitetura: {platform.machine()}\n"
                f"Python: {platform.python_version()}\n\n"
                f"Para informações detalhadas, instale: pip install psutil")
    
    def open_tools_menu(self):
        """Abre menu de ferramentas específicas da plataforma"""
        tools_window = tk.Toplevel(self.root)
        tools_window.title("Ferramentas do Sistema")
        tools_window.geometry("400x300")
        tools_window.transient(self.root)
        
        ttk.Label(tools_window, text="Ferramentas Disponíveis", 
                 font=('Arial', 12, 'bold')).pack(pady=10)
        
        # Ferramentas específicas da plataforma
        if self.platform == "windows":
            ttk.Button(tools_window, text="Abrir Event Viewer", 
                      command=lambda: os.system("eventvwr.msc")).pack(pady=5)
            ttk.Button(tools_window, text="Gerenciador de Serviços", 
                      command=lambda: os.system("services.msc")).pack(pady=5)
            ttk.Button(tools_window, text="Monitor de Rede", 
                      command=lambda: os.system("perfmon.exe /res")).pack(pady=5)
        elif self.platform == "darwin":
            ttk.Button(tools_window, text="Console do Sistema", 
                      command=lambda: os.system("open /Applications/Utilities/Console.app")).pack(pady=5)
            ttk.Button(tools_window, text="Monitor de Atividade", 
                      command=lambda: os.system("open /Applications/Utilities/Activity\\ Monitor.app")).pack(pady=5)
            ttk.Button(tools_window, text="Preferências de Rede", 
                      command=lambda: os.system("open /System/Library/PreferencePanes/Network.prefPane")).pack(pady=5)
        else:  # Linux/FreeBSD
            ttk.Button(tools_window, text="Monitor do Sistema", 
                      command=lambda: os.system("gnome-system-monitor 2>/dev/null || ksysguard 2>/dev/null &")).pack(pady=5)
            ttk.Button(tools_window, text="Configurações de Rede", 
                      command=lambda: os.system("nm-connection-editor 2>/dev/null &")).pack(pady=5)
            ttk.Button(tools_window, text="Logs do Sistema", 
                      command=lambda: os.system("gnome-logs 2>/dev/null || ksystemlog 2>/dev/null &")).pack(pady=5)
    
    # Manter os métodos existentes da classe anterior
    def refresh_data(self):
        """Atualiza todos os dados"""
        threading.Thread(target=self._refresh_data_thread, daemon=True).start()
    
    def _refresh_data_thread(self):
        """Thread para atualizar dados"""
        # Atualizar status
        status = self.api_request('/status')
        if status:
            self.status_data = status
            self.root.after(0, self.update_status_ui)
        
        # Atualizar redes
        networks = self.api_request('/network')
        if networks:
            self.networks_data = networks
            self.root.after(0, self.update_networks_ui)
        
        # Atualizar peers
        peers = self.api_request('/peer')
        if peers:
            self.peers_data = peers
            self.root.after(0, self.update_stats_ui)
    
    def update_status_ui(self):
        """Atualiza a UI com dados de status"""
        self.status_labels['status'].config(text="Online", fg='green')
        self.status_labels['version'].config(text=self.status_data.get('version', '-'))
        self.status_labels['node_id'].config(text=self.status_data.get('address', '-'))
        self.status_labels['address'].config(text=self.status_data.get('address', '-'))
        self.status_labels['online'].config(text="Sim" if self.status_data.get('online') else "Não")
        
        config = self.status_data.get('config', {}).get('settings', {})
        self.status_labels['port'].config(text=str(config.get('primaryPort', '-')))
        
        # Calcular uptime
        if 'clock' in self.status_data:
            uptime_seconds = (time.time() * 1000 - self.status_data['clock']) / 1000
            uptime_str = self.format_uptime(uptime_seconds)
            self.status_labels['uptime'].config(text=uptime_str)
    
    def update_networks_ui(self):
        """Atualiza a UI com dados de redes"""
        # Limpar treeview
        for item in self.networks_tree.get_children():
            self.networks_tree.delete(item)
        
        # Adicionar redes
        for network in self.networks_data:
            network_id = network.get('id', '')
            name = network.get('name', 'Sem nome')
            status = network.get('status', 'Desconhecido')
            assigned_ips = ', '.join(network.get('assignedAddresses', []))
            network_type = network.get('type', 'Privada')
            
            # Contar peers na mesma rede (simulado)
            peer_count = len([p for p in self.peers_data if 'paths' in p])
            
            self.networks_tree.insert('', tk.END, values=(
                network_id[:16], name, status, assigned_ips, network_type, peer_count
            ))
    
    def update_stats_ui(self):
        """Atualiza estatísticas"""
        self.stats_labels['networks'].config(text=str(len(self.networks_data)))
        self.stats_labels['peers'].config(text=str(len(self.peers_data)))
        self.stats_labels['last_update'].config(text=datetime.now().strftime('%H:%M:%S'))
        
        # Informações do planeta
        if 'planetWorldId' in self.status_data:
            self.stats_labels['planet_id'].config(text=str(self.status_data['planetWorldId'])[:8])
        
        # Configurações avançadas
        config = self.status_data.get('config', {}).get('settings', {})
        self.stats_labels['controller'].config(text="Local" if config.get('controller') else "Remoto")
        self.stats_labels['tcp_fallback'].config(text="Sim" if config.get('allowTcpFallbackRelay') else "Não")
        
        # Prefixo de interface (específico do sistema)
        if_prefixes = config.get('interfacePrefixBlacklist', [])
        if if_prefixes:
            self.stats_labels['if_prefix'].config(text=', '.join(if_prefixes[:2]))
        else:
            self.stats_labels['if_prefix'].config(text="Padrão")
    
    def join_network_dialog(self):
        """Dialog para entrar em uma rede"""
        network_id = simpledialog.askstring("Entrar em Rede", 
                                           "Digite o ID da rede (16 caracteres hex):")
        if network_id and len(network_id) == 16:
            self.join_network(network_id)
        elif network_id:
            messagebox.showerror("Erro", "ID da rede deve ter exatamente 16 caracteres hexadecimais")
    
    def join_network(self, network_id):
        """Entrar em uma rede"""
        def _join():
            result = self.api_request(f'/network/{network_id}', method='POST')
            if result:
                self.root.after(0, lambda: messagebox.showinfo("Sucesso", "Conectado à rede com sucesso!"))
                self.root.after(0, self.refresh_data)
            else:
                self.root.after(0, lambda: messagebox.showerror("Erro", "Falha ao conectar à rede"))
        
        threading.Thread(target=_join, daemon=True).start()
    
    def leave_network(self):
        """Sair da rede selecionada"""
        selection = self.networks_tree.selection()
        if not selection:
            messagebox.showwarning("Aviso", "Selecione uma rede para sair")
            return
        
        item = self.networks_tree.item(selection[0])
        network_id = item['values'][0]
        
        # Buscar ID completo
        full_network_id = None
        for network in self.networks_data:
            if network.get('id', '').startswith(network_id):
                full_network_id = network.get('id')
                break
        
        if not full_network_id:
            messagebox.showerror("Erro", "ID da rede não encontrado")
            return
        
        if messagebox.askyesno("Confirmar", f"Sair da rede {full_network_id}?"):
            def _leave():
                result = self.api_request(f'/network/{full_network_id}', method='DELETE')
                if result:
                    self.root.after(0, lambda: messagebox.showinfo("Sucesso", "Desconectado da rede com sucesso!"))
                    self.root.after(0, self.refresh_data)
                else:
                    self.root.after(0, lambda: messagebox.showerror("Erro", "Falha ao sair da rede"))
            
            threading.Thread(target=_leave, daemon=True).start()
    
    def show_network_details(self):
        """Mostrar detalhes da rede selecionada"""
        selection = self.networks_tree.selection()
        if not selection:
            messagebox.showwarning("Aviso", "Selecione uma rede para ver detalhes")
            return
        
        item = self.networks_tree.item(selection[0])
        network_id = item['values'][0]
        
        # Buscar dados completos da rede
        network_data = None
        for network in self.networks_data:
            if network.get('id', '').startswith(network_id):
                network_data = network
                break
        
        if network_data:
            details = f"""
ID da Rede: {network_data.get('id', '-')}
Nome: {network_data.get('name', 'Sem nome')}
Status: {network_data.get('status', 'Desconhecido')}
Tipo: {network_data.get('type', 'Privada')}
IPs Atribuídos: {', '.join(network_data.get('assignedAddresses', []))}
MAC: {network_data.get('mac', '-')}
MTU: {network_data.get('mtu', '-')}
Broadcast: {'Sim' if network_data.get('broadcastEnabled') else 'Não'}
Bridge: {'Sim' if network_data.get('bridge') else 'Não'}
            """
            messagebox.showinfo("Detalhes da Rede", details.strip())
    
    def show_peers(self):
        """Mostrar janela de peers"""
        peers_window = tk.Toplevel(self.root)
        peers_window.title("Peers Conectados")
        peers_window.geometry("800x500")
        peers_window.transient(self.root)
        
        # Treeview para peers
        peer_columns = ('Address', 'Latency', 'Version', 'Paths', 'Role')
        peer_tree = ttk.Treeview(peers_window, columns=peer_columns, show='headings')
        
        for col in peer_columns:
            peer_tree.heading(col, text=col)
            peer_tree.column(col, width=150)
        
        # Scrollbar para peers
        peer_scrollbar = ttk.Scrollbar(peers_window, orient=tk.VERTICAL, command=peer_tree.yview)
        peer_tree.configure(yscrollcommand=peer_scrollbar.set)
        
        peer_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        peer_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        # Carregar dados dos peers
        for peer in self.peers_data:
            address = peer.get('address', '-')
            latency = f"{peer.get('latency', 0)}ms"
            version = peer.get('version', '-')
            paths_count = len(peer.get('paths', []))
            role = peer.get('role', 'LEAF')
            
            peer_tree.insert('', tk.END, values=(address, latency, version, paths_count, role))
    
    def format_uptime(self, seconds):
        """Formata tempo de uptime"""
        if seconds < 0:
            return "-"
        
        days = int(seconds // 86400)
        hours = int((seconds % 86400) // 3600)
        minutes = int((seconds % 3600) // 60)
        
        if days > 0:
            return f"{days}d {hours}h {minutes}m"
        elif hours > 0:
            return f"{hours}h {minutes}m"
        else:
            return f"{minutes}m"
    
    def start_auto_refresh(self):
        """Inicia atualização automática"""
        def auto_refresh():
            while True:
                time.sleep(30)  # Atualizar a cada 30 segundos
                self.refresh_data()
        
        threading.Thread(target=auto_refresh, daemon=True).start()
    
    def run(self):
        """Executa a aplicação"""
        if not self.auth_token:
            messagebox.showerror("Erro", "Não foi possível iniciar sem token de autenticação")
            return
        
        self.root.mainloop()

if __name__ == "__main__":
    app = ZeroTierCrossPlatformManager()
    app.run()
