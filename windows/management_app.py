# ZeroTier Management Application
# Aplicação desktop Windows para gestão do ZeroTier

import tkinter as tk
from tkinter import ttk, messagebox, simpledialog
import requests
import json
import threading
import time
from datetime import datetime, timedelta
import os
import sys

class ZeroTierManager:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("ZeroTier Management Console")
        self.root.geometry("1000x700")
        self.root.configure(bg='#f0f0f0')
        
        # Configuração da API
        self.api_base = "http://localhost:9993"
        self.auth_token = self.get_auth_token()
        
        # Dados
        self.status_data = {}
        self.networks_data = []
        self.peers_data = []
        
        self.setup_ui()
        self.start_auto_refresh()
        
    def get_auth_token(self):
        """Obtém o token de autenticação do arquivo authtoken.secret"""
        try:
            # Caminho típico no Windows
            token_path = os.path.expanduser("~/AppData/Local/ZeroTier/One/authtoken.secret")
            if not os.path.exists(token_path):
                # Caminho alternativo
                token_path = "C:/ProgramData/ZeroTier/One/authtoken.secret"
            
            if os.path.exists(token_path):
                with open(token_path, 'r') as f:
                    return f.read().strip()
            else:
                messagebox.showerror("Erro", "Token de autenticação não encontrado")
                return None
        except Exception as e:
            messagebox.showerror("Erro", f"Erro ao ler token: {e}")
            return None
    
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
        # Estilo
        style = ttk.Style()
        style.theme_use('clam')
        
        # Frame principal
        main_frame = ttk.Frame(self.root, padding="10")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        
        # Configurar grid
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(0, weight=1)
        main_frame.columnconfigure(1, weight=1)
        main_frame.rowconfigure(2, weight=1)
        
        # Título
        title_label = tk.Label(main_frame, text="🌐 ZeroTier Management Console", 
                              font=('Arial', 16, 'bold'), bg='#f0f0f0')
        title_label.grid(row=0, column=0, columnspan=2, pady=(0, 20))
        
        # Frame de status (esquerda)
        status_frame = ttk.LabelFrame(main_frame, text="Status do Serviço", padding="10")
        status_frame.grid(row=1, column=0, sticky=(tk.W, tk.E, tk.N), padx=(0, 10))
        
        # Labels de status
        self.status_labels = {}
        status_fields = [
            ("Estado:", "status"),
            ("Versão:", "version"),
            ("Node ID:", "node_id"),
            ("Endereço:", "address"),
            ("Porta:", "port"),
            ("Uptime:", "uptime")
        ]
        
        for i, (label, key) in enumerate(status_fields):
            tk.Label(status_frame, text=label, font=('Arial', 9, 'bold')).grid(
                row=i, column=0, sticky=tk.W, pady=2)
            self.status_labels[key] = tk.Label(status_frame, text="-", font=('Arial', 9))
            self.status_labels[key].grid(row=i, column=1, sticky=tk.W, padx=(10, 0), pady=2)
        
        # Frame de estatísticas (direita)
        stats_frame = ttk.LabelFrame(main_frame, text="Estatísticas", padding="10")
        stats_frame.grid(row=1, column=1, sticky=(tk.W, tk.E, tk.N))
        
        # Labels de estatísticas
        self.stats_labels = {}
        stats_fields = [
            ("Redes Ativas:", "networks"),
            ("Peers Conectados:", "peers"),
            ("Última Atualização:", "last_update")
        ]
        
        for i, (label, key) in enumerate(stats_fields):
            tk.Label(stats_frame, text=label, font=('Arial', 9, 'bold')).grid(
                row=i, column=0, sticky=tk.W, pady=2)
            self.stats_labels[key] = tk.Label(stats_frame, text="-", font=('Arial', 9))
            self.stats_labels[key].grid(row=i, column=1, sticky=tk.W, padx=(10, 0), pady=2)
        
        # Frame de redes
        networks_frame = ttk.LabelFrame(main_frame, text="Redes Conectadas", padding="10")
        networks_frame.grid(row=2, column=0, columnspan=2, sticky=(tk.W, tk.E, tk.N, tk.S), pady=(20, 0))
        networks_frame.columnconfigure(0, weight=1)
        networks_frame.rowconfigure(0, weight=1)
        
        # Treeview para redes
        columns = ('ID', 'Nome', 'Status', 'IP', 'Tipo')
        self.networks_tree = ttk.Treeview(networks_frame, columns=columns, show='headings', height=10)
        
        # Configurar colunas
        self.networks_tree.heading('ID', text='ID da Rede')
        self.networks_tree.heading('Nome', text='Nome')
        self.networks_tree.heading('Status', text='Status')
        self.networks_tree.heading('IP', text='IP Atribuído')
        self.networks_tree.heading('Tipo', text='Tipo')
        
        self.networks_tree.column('ID', width=150)
        self.networks_tree.column('Nome', width=150)
        self.networks_tree.column('Status', width=100)
        self.networks_tree.column('IP', width=150)
        self.networks_tree.column('Tipo', width=100)
        
        # Scrollbar
        scrollbar = ttk.Scrollbar(networks_frame, orient=tk.VERTICAL, command=self.networks_tree.yview)
        self.networks_tree.configure(yscrollcommand=scrollbar.set)
        
        self.networks_tree.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))
        scrollbar.grid(row=0, column=1, sticky=(tk.N, tk.S))
        
        # Frame de botões
        buttons_frame = ttk.Frame(networks_frame)
        buttons_frame.grid(row=1, column=0, columnspan=2, pady=(10, 0))
        
        ttk.Button(buttons_frame, text="🔄 Atualizar", command=self.refresh_data).pack(side=tk.LEFT, padx=(0, 10))
        ttk.Button(buttons_frame, text="➕ Entrar em Rede", command=self.join_network_dialog).pack(side=tk.LEFT, padx=(0, 10))
        ttk.Button(buttons_frame, text="❌ Sair da Rede", command=self.leave_network).pack(side=tk.LEFT, padx=(0, 10))
        ttk.Button(buttons_frame, text="ℹ️ Detalhes", command=self.show_network_details).pack(side=tk.LEFT)
        
        # Carregar dados iniciais
        self.refresh_data()
    
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
            
            self.networks_tree.insert('', tk.END, values=(
                network_id[:16], name, status, assigned_ips, network_type
            ))
    
    def update_stats_ui(self):
        """Atualiza estatísticas"""
        self.stats_labels['networks'].config(text=str(len(self.networks_data)))
        self.stats_labels['peers'].config(text=str(len(self.peers_data)))
        self.stats_labels['last_update'].config(text=datetime.now().strftime('%H:%M:%S'))
    
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
    app = ZeroTierManager()
    app.run()
