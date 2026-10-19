# ESP32Safe – Cofre eletrônico com ESP32 e HiveMQ (CP2)

FIAP – TDS | Disciplina: Disruptive Architectures: IoT, IoB & Generative AI
Professor: Dr. Marcel Stefan Wagner | Turma: 1TDSPS | Grupo MQTT: `grupo-k7q2`

## Link público do Wokwi
https://wokwi.com/projects/478258677728303105

## Link público do GitHub
https://github.com/GustavoSiciliano/CP2_IOT

## Integrantes
| Nome completo | RM |
|---|---|
| Gustavo de Jesus | 567926 |
| Gustavo Siciliano | 568419 |
| Samuel Keniti | 567614 |
| Manuel Pedro | 567458 |
| Vinicius Morrone | 566884 |

## Conteúdo do .zip
| Item | Descrição |
|---|---|
| `Relatorio_Tecnico_CP2.pdf` | Relatório técnico |
| `wokwi/` | `sketch.ino`, `SafeState.h/.cpp`, `Icons.h/.cpp`, `diagram.json`, `libraries.txt`, `wokwi-project.txt` |
| `circuito/` | Captura do circuito completo |
| `evidencias/` | Capturas dos testes (Wokwi, Monitor Serial e cliente HiveMQ); `FigNN` = número da figura no relatório |
| `LICENSE-REFERENCIA.txt` | Licença MIT do código de referência (Uri Shaked, 2020) |

## Como executar a simulação
1. Abra o cliente web https://www.hivemq.com/demos/websocket-client/ e preencha **Host** `broker.hivemq.com`, **Port** `8884` (WebSocket), **SSL** ligado. Clique em **Connect**.
2. Em **Add New Topic Subscription**, assine `fiap/DA/CP2/1tdsps/grupo-k7q2/cofre/estado`.
3. Abra o link do Wokwi e clique em **Play**. No Monitor Serial devem aparecer `MQTT conectado` e `Enviado: Unlocked` (ou `Locked`).
4. Para o teste do segundo assinante, abra outra aba do cliente web com **outro Client ID** e assine o mesmo tópico.

## Configurações MQTT
| Parâmetro | Valor |
|---|---|
| Wi-Fi (Wokwi) | `Wokwi-GUEST`, sem senha |
| Broker | `broker.hivemq.com` (público) |
| Porta do ESP32 | 1883 (MQTT sobre TCP), sem TLS, sem usuário e senha |
| Cliente web | porta 8884 (WebSocket), SSL ligado |
| Tópico | `fiap/DA/CP2/1tdsps/grupo-k7q2/cofre/estado` |
| Payload | `Locked` ou `Unlocked` (texto simples) |
| Retain | sim |
| Client ID do ESP32 | `cofre-esp32-` + hexadecimal aleatório |

## Sequência de operação do teclado
O projeto não traz senha predefinida. Senha de demonstração (**fictícia**): `1234`.

1. **Primeira execução** (sem senha): o cofre inicia destrancado. Pressione `#`.
2. `Enter new code:` → `1234`; `Confirm new code` → `1234`. O cofre tranca (servo 20°, LCD "Safe Locked!", publica `Locked`).
3. **Senha incorreta** (ex.: `0000`): `Access Denied!`, continua `Locked`, nada é publicado como `Unlocked`.
4. **Senha correta** (`1234`): `Unlocked!`, servo 90°, publica `Unlocked`.
5. Destrancado: `#` tranca novamente; `A` define nova senha (com confirmação).
6. **Confirmação diferente** no cadastro: `Code mismatch / Safe not locked!`, o cofre não tranca.
7. **Queda do MQTT:** no Monitor Serial digite `d` + Enter. O MQTT é desligado por 20 s (teclado e LCD continuam funcionando); depois o ESP32 reconecta e republica o estado atual.

## Observações
- O estado publicado é o da trava **comandada pelo programa**; não há sensor que comprove a porta fechada.
- Senha e estado ficam na flash (Preferences/NVS); a senha **nunca** é enviada ao MQTT. No Wokwi, a flash **não persiste** entre reinícios da simulação (o cofre reinicia destrancado e sem senha); em placa real ela persiste.
- O broker é público: usar apenas dados de simulação.
- Código baseado em "Arduino Electronic Safe", Copyright (C) 2020, Uri Shaked, licença MIT.
