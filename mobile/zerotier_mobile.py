# ZeroTier Mobile Manager - Kivy Cross-Platform App
# Suporte para Android, iOS, Windows Mobile, etc.

from kivy.app import App
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout
from kivy.uix.label import Label
from kivy.uix.button import Button
from kivy.uix.textinput import TextInput
from kivy.uix.popup import Popup
from kivy.uix.scrollview import ScrollView
from kivy.clock import Clock
from kivy.utils import platform
import requests
import json
import os

class ZeroTierMobileManager(BoxLayout):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.orientation = 'vertical'
        self.padding = 20
        self.spacing = 10
        
        # Configuração da API
        self.api_base = "http://localhost:9993"
        self.auth_token = self.get_auth_token()
        
        # Dados
        self.networks_data = []
        self.peers_data = []
        
        self.build_ui()
        self.schedule_refresh()
    
    def get_auth_token(self):
        """Obtém token baseado na plataforma mobile"""
        token_paths = []
        
        if platform == 'android':
            # Android paths
            token_paths = [
                '/sdcard/zerotier/authtoken.secret',
                '/data/data/com.zerotier.one/authtoken.secret',
                '/storage/emulated/0/Android/data/com.zerotier.one/files/authtoken.secret'
            ]
        elif platform == 'ios':
            # iOS paths (sandboxed)
            token_paths = [
                'Documents/authtoken.secret',
                'Library/Application Support/ZeroTier/authtoken.secret'
            ]
        else:
            # Desktop fallback
            token_paths = [
                os.path.expanduser('~/.zerotier/authtoken.secret'),
                '/var/lib/zerotier-one/authtoken.secret'
            ]
        
        for path in token_paths:
            try:
                if os.path.exists(path):
                    with open(path, 'r') as f:
                        return f.read().strip()
            except:
                continue
        
        # Fallback para entrada manual
        return self.prompt_token()
    
    def prompt_token(self):
        """Prompt para entrada manual do token"""
        content = BoxLayout(orientation='vertical', spacing=10, padding=20)
        
        content.add_widget(Label(text='Digite o token de autenticação ZeroTier:',
                                text_size=(None, None), halign='center'))
        
        token_input = TextInput(multiline=False, password=True, size_hint_y=None, height=40)
        content.add_widget(token_input)
        
        buttons = BoxLayout(size_hint_y=None, height=50, spacing=10)
        
        ok_btn = Button(text='OK')
        cancel_btn = Button(text='Cancelar')
        
        buttons.add_widget(ok_btn)
        buttons.add_widget(cancel_btn)
        content.add_widget(buttons)
        
        popup = Popup(title='Configuração do Token',
                     content=content,
                     size_hint=(0.9, 0.5))
        
        self.token_result = None
        
        def on_ok(instance):
            self.token_result = token_input.text
            popup.dismiss()
        
        def on_cancel(instance):
            self.token_result = None
            popup.dismiss()
        
        ok_btn.bind(on_press=on_ok)
        cancel_btn.bind(on_press=on_cancel)
        
        popup.open()
        return self.token_result
    
    def build_ui(self):
        """Constrói a interface mobile-friendly"""
        # Header
        header = BoxLayout(size_hint_y=None, height=80, padding=10)
        header.add_widget(Label(text='📱 ZeroTier Mobile Manager',
                               font_size=20, bold=True))
        self.add_widget(header)
        
        # Status Card
        status_card = self.create_card("📊 Status", self.build_status_section())
        self.add_widget(status_card)
        
        # Networks Card  
        networks_card = self.create_card("🌐 Redes", self.build_networks_section())
        self.add_widget(networks_card)
        
        # Actions Card
        actions_card = self.create_card("⚡ Ações", self.build_actions_section())
        self.add_widget(actions_card)
    
    def create_card(self, title, content):
        """Cria um card com título e conteúdo"""
        card = BoxLayout(orientation='vertical', 
                        size_hint_y=None, 
                        height=200,
                        padding=10,
                        spacing=5)
        
        # Título do card
        title_label = Label(text=title, 
                           font_size=16, 
                           bold=True,
                           size_hint_y=None, 
                           height=30)
        card.add_widget(title_label)
        
        # Conteúdo
        card.add_widget(content)
        
        return card
    
    def build_status_section(self):
        """Seção de status do sistema"""
        layout = GridLayout(cols=2, spacing=5)
        
        # Labels de status
        self.status_labels = {}
        status_items = [
            ('Estado:', 'status'),
            ('Versão:', 'version'), 
            ('Node ID:', 'node_id'),
            ('Redes:', 'networks_count')
        ]
        
        for label_text, key in status_items:
            layout.add_widget(Label(text=label_text, 
                                   text_size=(None, None),
                                   halign='left'))
            
            status_label = Label(text='-', 
                               text_size=(None, None),
                               halign='left')
            self.status_labels[key] = status_label
            layout.add_widget(status_label)
        
        return layout
    
    def build_networks_section(self):
        """Seção de redes"""
        layout = BoxLayout(orientation='vertical')
        
        # Lista de redes (ScrollView para mobile)
        scroll = ScrollView()
        self.networks_list = BoxLayout(orientation='vertical', 
                                      size_hint_y=None,
                                      spacing=5)
        self.networks_list.bind(minimum_height=self.networks_list.setter('height'))
        
        scroll.add_widget(self.networks_list)
        layout.add_widget(scroll)
        
        return layout
    
    def build_actions_section(self):
        """Seção de ações"""
        layout = GridLayout(cols=2, spacing=10)
        
        # Botões de ação mobile-friendly
        btn_refresh = Button(text='🔄\nAtualizar', 
                           size_hint_y=None, 
                           height=60)
        btn_refresh.bind(on_press=self.refresh_data)
        
        btn_join = Button(text='➕\nEntrar', 
                         size_hint_y=None, 
                         height=60)
        btn_join.bind(on_press=self.show_join_dialog)
        
        btn_qr = Button(text='📱\nQR Code', 
                       size_hint_y=None, 
                       height=60)
        btn_qr.bind(on_press=self.show_qr_code)
        
        btn_share = Button(text='📤\nCompartilhar', 
                          size_hint_y=None, 
                          height=60)
        btn_share.bind(on_press=self.share_config)
        
        layout.add_widget(btn_refresh)
        layout.add_widget(btn_join) 
        layout.add_widget(btn_qr)
        layout.add_widget(btn_share)
        
        return layout
    
    def api_request(self, endpoint, method='GET', data=None):
        """Requisição para API ZeroTier"""
        try:
            headers = {
                'X-ZT1-Auth': self.auth_token,
                'Content-Type': 'application/json'
            }
            
            url = f"{self.api_base}{endpoint}"
            
            if method == 'GET':
                response = requests.get(url, headers=headers, timeout=10)
            elif method == 'POST':
                response = requests.post(url, headers=headers, json=data, timeout=10)
            elif method == 'DELETE':
                response = requests.delete(url, headers=headers, timeout=10)
            
            if response.status_code == 200:
                return response.json()
            
        except Exception as e:
            print(f"API Error: {e}")
        
        return None
    
    def refresh_data(self, instance=None):
        """Atualiza dados do ZeroTier"""
        # Status
        status = self.api_request('/status')
        if status:
            self.status_labels['status'].text = 'Online' if status.get('online') else 'Offline'
            self.status_labels['version'].text = status.get('version', '-')
            self.status_labels['node_id'].text = status.get('address', '-')[:8]
        
        # Networks
        networks = self.api_request('/network')
        if networks:
            self.networks_data = networks
            self.status_labels['networks_count'].text = str(len(networks))
            self.update_networks_ui()
    
    def update_networks_ui(self):
        """Atualiza lista de redes na UI"""
        self.networks_list.clear_widgets()
        
        for network in self.networks_data:
            # Card para cada rede
            net_card = BoxLayout(orientation='horizontal',
                               size_hint_y=None,
                               height=80,
                               spacing=10,
                               padding=5)
            
            # Info da rede
            info_layout = BoxLayout(orientation='vertical')
            
            name = network.get('name', 'Sem nome')
            network_id = network.get('id', '')[:8]
            status = network.get('status', 'Desconhecido')
            
            info_layout.add_widget(Label(text=f"📡 {name}",
                                       text_size=(None, None),
                                       halign='left'))
            info_layout.add_widget(Label(text=f"ID: {network_id}",
                                       text_size=(None, None), 
                                       halign='left'))
            info_layout.add_widget(Label(text=f"Status: {status}",
                                       text_size=(None, None),
                                       halign='left'))
            
            net_card.add_widget(info_layout)
            
            # Botão de ações
            actions_layout = BoxLayout(orientation='vertical',
                                     size_hint_x=None,
                                     width=100)
            
            details_btn = Button(text='ℹ️', 
                               size_hint_y=None, 
                               height=35)
            details_btn.bind(on_press=lambda x, net=network: self.show_network_details(net))
            
            leave_btn = Button(text='❌',
                             size_hint_y=None,
                             height=35)
            leave_btn.bind(on_press=lambda x, net=network: self.leave_network(net))
            
            actions_layout.add_widget(details_btn)
            actions_layout.add_widget(leave_btn)
            
            net_card.add_widget(actions_layout)
            self.networks_list.add_widget(net_card)
    
    def show_join_dialog(self, instance):
        """Dialog para entrar em rede"""
        content = BoxLayout(orientation='vertical', spacing=10, padding=20)
        
        content.add_widget(Label(text='Digite o ID da rede (16 caracteres):'))
        
        network_input = TextInput(multiline=False, 
                                 size_hint_y=None, 
                                 height=40,
                                 input_filter='hex')
        content.add_widget(network_input)
        
        buttons = BoxLayout(size_hint_y=None, height=50, spacing=10)
        
        join_btn = Button(text='Entrar')
        cancel_btn = Button(text='Cancelar')
        
        buttons.add_widget(join_btn)
        buttons.add_widget(cancel_btn)
        content.add_widget(buttons)
        
        popup = Popup(title='Entrar em Rede',
                     content=content,
                     size_hint=(0.9, 0.6))
        
        def on_join(instance):
            network_id = network_input.text.strip()
            if len(network_id) == 16:
                self.join_network(network_id)
                popup.dismiss()
            else:
                self.show_error("ID deve ter 16 caracteres hexadecimais")
        
        def on_cancel(instance):
            popup.dismiss()
        
        join_btn.bind(on_press=on_join)
        cancel_btn.bind(on_press=on_cancel)
        
        popup.open()
    
    def join_network(self, network_id):
        """Entrar em uma rede"""
        result = self.api_request(f'/network/{network_id}', method='POST')
        if result:
            self.show_success("Conectado à rede com sucesso!")
            self.refresh_data()
        else:
            self.show_error("Falha ao conectar à rede")
    
    def leave_network(self, network):
        """Sair de uma rede"""
        network_id = network.get('id', '')
        network_name = network.get('name', 'Sem nome')
        
        # Confirmar ação
        content = BoxLayout(orientation='vertical', spacing=10, padding=20)
        content.add_widget(Label(text=f'Sair da rede "{network_name}"?'))
        
        buttons = BoxLayout(size_hint_y=None, height=50, spacing=10)
        
        leave_btn = Button(text='Sair')
        cancel_btn = Button(text='Cancelar')
        
        buttons.add_widget(leave_btn)
        buttons.add_widget(cancel_btn)
        content.add_widget(buttons)
        
        popup = Popup(title='Confirmar',
                     content=content,
                     size_hint=(0.8, 0.4))
        
        def on_leave(instance):
            result = self.api_request(f'/network/{network_id}', method='DELETE')
            if result:
                self.show_success("Desconectado da rede!")
                self.refresh_data()
            else:
                self.show_error("Falha ao sair da rede")
            popup.dismiss()
        
        def on_cancel(instance):
            popup.dismiss()
        
        leave_btn.bind(on_press=on_leave)
        cancel_btn.bind(on_press=on_cancel)
        
        popup.open()
    
    def show_network_details(self, network):
        """Mostrar detalhes da rede"""
        details = f"""
ID: {network.get('id', '-')}
Nome: {network.get('name', 'Sem nome')}
Status: {network.get('status', 'Desconhecido')}
Tipo: {network.get('type', 'Privada')}
IPs: {', '.join(network.get('assignedAddresses', []))}
MAC: {network.get('mac', '-')}
MTU: {network.get('mtu', '-')}
        """.strip()
        
        content = ScrollView()
        content.add_widget(Label(text=details, 
                               text_size=(None, None),
                               halign='left'))
        
        popup = Popup(title='Detalhes da Rede',
                     content=content,
                     size_hint=(0.9, 0.7))
        popup.open()
    
    def show_qr_code(self, instance):
        """Mostrar QR code para configuração"""
        try:
            import qrcode
            from io import BytesIO
            from kivy.uix.image import Image
            
            # Gerar dados de configuração
            config_data = {
                'api_base': self.api_base,
                'auth_token': self.auth_token[:8] + '...',  # Parcial por segurança
                'networks': [{'id': n.get('id'), 'name': n.get('name')} 
                           for n in self.networks_data]
            }
            
            # Gerar QR code
            qr = qrcode.QRCode(version=1, box_size=10, border=5)
            qr.add_data(json.dumps(config_data))
            qr.make(fit=True)
            
            img = qr.make_image(fill_color="black", back_color="white")
            
            # Converter para exibição no Kivy
            bio = BytesIO()
            img.save(bio, format='PNG')
            bio.seek(0)
            
            content = BoxLayout(orientation='vertical', padding=20)
            content.add_widget(Label(text='QR Code para Configuração:', 
                                   size_hint_y=None, height=30))
            
            # TODO: Implementar exibição de imagem QR no Kivy
            content.add_widget(Label(text='QR Code gerado!\n(Implementar exibição)'))
            
            popup = Popup(title='QR Code',
                         content=content,
                         size_hint=(0.8, 0.8))
            popup.open()
            
        except ImportError:
            self.show_error("Instale 'qrcode' para gerar QR codes")
    
    def share_config(self, instance):
        """Compartilhar configuração"""
        try:
            from plyer import filechooser
            
            # Gerar arquivo de configuração
            config = {
                'zerotier_networks': [
                    {
                        'id': network.get('id'),
                        'name': network.get('name'),
                        'status': network.get('status')
                    }
                    for network in self.networks_data
                ]
            }
            
            config_text = json.dumps(config, indent=2)
            
            # Salvar e compartilhar
            # TODO: Implementar compartilhamento nativo por plataforma
            self.show_success("Configuração preparada para compartilhamento")
            
        except ImportError:
            self.show_error("Funcionalidade de compartilhamento não disponível")
    
    def show_success(self, message):
        """Exibir mensagem de sucesso"""
        popup = Popup(title='Sucesso',
                     content=Label(text=message),
                     size_hint=(0.8, 0.3))
        popup.open()
        Clock.schedule_once(lambda dt: popup.dismiss(), 2)
    
    def show_error(self, message):
        """Exibir mensagem de erro"""
        popup = Popup(title='Erro',
                     content=Label(text=message),
                     size_hint=(0.8, 0.3))
        popup.open()
        Clock.schedule_once(lambda dt: popup.dismiss(), 3)
    
    def schedule_refresh(self):
        """Agendar atualização automática"""
        self.refresh_data()
        Clock.schedule_interval(self.refresh_data, 30)  # A cada 30 segundos

class ZeroTierMobileApp(App):
    def build(self):
        return ZeroTierMobileManager()
    
    def on_start(self):
        """Executado quando o app inicia"""
        # Configurações específicas mobile
        if platform == 'android':
            from android.permissions import request_permissions, Permission
            request_permissions([
                Permission.INTERNET,
                Permission.ACCESS_NETWORK_STATE,
                Permission.WRITE_EXTERNAL_STORAGE
            ])

if __name__ == '__main__':
    ZeroTierMobileApp().run()
