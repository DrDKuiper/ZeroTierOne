# ZeroTier Management Interface

Este projeto adiciona interfaces de gestão profissionais ao ZeroTier One.

## 📋 Opções Disponíveis

### 1. 🌐 Interface Web (RECOMENDADO)

**Recursos:**
- Interface moderna e responsiva
- Dashboard em tempo real
- Gestão de redes e peers
- API REST integrada
- Autenticação por token

**Como usar:**
1. Habilite o servidor web no ZeroTier (por padrão já está habilitado)
2. Abra o arquivo `webui/index.html` em um navegador
3. Configure o token de autenticação no código JavaScript

**Configuração no local.conf:**
```json
{
  "settings": {
    "enableWebServer": true,
    "allowManagementFrom": ["127.0.0.1/8"],
    "primaryPort": 9993
  }
}
```

### 2. 🖥️ Aplicação Desktop Windows

**Recursos:**
- Interface nativa Windows
- Gestão completa via GUI
- Notificações do sistema
- Auto-atualização de dados
- Integração com token local

**Como compilar:**
1. Instale Python 3.7+
2. Execute o script PowerShell:
   ```powershell
   .\Build-ZeroTierManagement.ps1
   ```
3. Execute `install.bat` como administrador

**Requisitos:**
- Python 3.7+
- Bibliotecas: `tkinter`, `requests`
- ZeroTier One rodando

## 🔧 Configuração

### Token de Autenticação

O ZeroTier gera automaticamente um token em:
- **Windows:** `%LOCALAPPDATA%\ZeroTier\One\authtoken.secret`
- **Alternativo:** `C:\ProgramData\ZeroTier\One\authtoken.secret`

### API Endpoints Disponíveis

| Endpoint | Método | Descrição |
|----------|--------|-----------|
| `/status` | GET | Status geral do serviço |
| `/network` | GET | Lista todas as redes |
| `/network/{id}` | POST | Juntar-se a uma rede |
| `/network/{id}` | DELETE | Sair de uma rede |
| `/peer` | GET | Lista todos os peers |
| `/config` | GET/POST | Configurações |

### Configuração de Segurança

Para acesso remoto, configure `allowManagementFrom`:

```json
{
  "settings": {
    "allowManagementFrom": [
      "127.0.0.1/8",      # Local apenas
      "10.0.0.0/8",       # Rede privada
      "192.168.0.0/16"    # Rede local
    ]
  }
}
```

## 🚀 Build e Deploy

### Interface Web
```bash
# Copiar arquivos para diretório do ZeroTier
cp -r webui/ /var/lib/zerotier-one/app/

# Ou no Windows
xcopy webui C:\ProgramData\ZeroTier\One\app\ /E /I
```

### Aplicação Desktop
```powershell
# Compilar para executável
.\Build-ZeroTierManagement.ps1

# Instalar
cd dist
.\install.bat
```

## 📱 Recursos Avançados

### 1. Dashboard Personalizado
- Métricas em tempo real
- Gráficos de performance
- Alertas e notificações
- Logs de eventos

### 2. Gestão Multi-Node
- Controle de múltiplos nós
- Sincronização de configurações
- Deploy automático

### 3. Integração com Active Directory
- Autenticação SSO
- Políticas de grupo
- Auditoria centralizada

## 🔒 Segurança

### Melhores Práticas:
1. **Token Security:** Nunca expor o token publicamente
2. **Network Access:** Limitar IPs autorizados
3. **HTTPS:** Usar proxy reverso com SSL para produção
4. **Firewall:** Bloquear porta 9993 externamente se não necessário

### Configuração para Produção:
```nginx
# Nginx reverse proxy
server {
    listen 443 ssl;
    server_name zerotier.exemplo.com;
    
    ssl_certificate /path/to/cert.pem;
    ssl_certificate_key /path/to/key.pem;
    
    location / {
        proxy_pass http://127.0.0.1:9993;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header Authorization "Bearer YOUR_TOKEN_HERE";
    }
}
```

## 🛠️ Desenvolvimento

### Estrutura do Projeto:
```
├── webui/                 # Interface web
│   ├── index.html        # Dashboard principal
│   ├── assets/           # CSS, JS, imagens
│   └── api/              # Scripts da API
├── windows/              # Aplicação Windows
│   ├── management_app.py # App principal
│   └── Build-*.ps1       # Scripts de build
└── docs/                 # Documentação
```

### Adicionar Novos Recursos:
1. Estender a API REST no `OneService.cpp`
2. Atualizar interface web ou desktop
3. Testar com diferentes configurações
4. Documentar mudanças

## 📞 Suporte

Para problemas ou sugestões:
1. Verificar logs do ZeroTier: `zerotier-cli info`
2. Testar conectividade: `curl -H "X-ZT1-Auth: TOKEN" http://localhost:9993/status`
3. Verificar configurações de firewall
4. Consultar documentação oficial do ZeroTier

## 📄 Licença

Este projeto segue a mesma licença do ZeroTier One.
