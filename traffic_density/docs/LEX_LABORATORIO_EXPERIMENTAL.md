# Laboratório Experimental (LEX) - Sistema de Processamento Visual
## Sistema de Detecção de Densidade de Tráfego por Câmeras de Segurança

---

## 📋 Sumário

1. [Introdução](#introdução)
2. [Aplicações do Sistema](#aplicações-do-sistema)
3. [Procedimento Experimental](#procedimento-experimental)
4. [Questionário de Avaliação](#questionário-de-avaliação)
5. [Enquete Subjetiva de Opinião (ESO)](#enquete-subjetiva-de-opinião-eso)
6. [Critérios de Avaliação](#critérios-de-avaliação)

---

## Introdução

### O que é o Sistema?

O **Sistema de Detecção de Densidade de Tráfego** é uma aplicação de visão computacional que analisa imagens de câmeras de segurança para detectar veículos e calcular a densidade do tráfego em tempo real. O sistema utiliza técnicas de inteligência artificial (rede neural YOLOv3) para identificar carros, motos, ônibus e caminhões, gerando relatórios automáticos sobre as condições do trânsito.

### Como Funciona?

O sistema opera em 4 etapas principais:

```
┌─────────────────┐      ┌──────────────────┐      ┌─────────────────┐      ┌──────────────┐
│  1. CAPTURA     │  →   │  2. PRÉ-         │  →   │  3. DETECÇÃO    │  →   │  4. RELATÓRIO│
│  DE IMAGEM      │      │  PROCESSAMENTO   │      │  DE VEÍCULOS    │      │  E ANÁLISE   │
└─────────────────┘      └──────────────────┘      └─────────────────┘      └──────────────┘
   Câmera              Filtros de Imagem         YOLOv3 (IA)           Densidade de Tráfego
   Pública                  CLAHE                    OpenCV                   JSON Report
```

### Interfaces do Sistema

#### Entrada (Input)
- **Fonte de dados**: Stream de vídeo de câmera pública (Santo André, SP - Avenida dos Estados)
- **Formato**: Imagens JPEG capturadas frame a frame
- **Localização**: Armazenadas em `resources/images/avenida_dos_estados/`

#### Processamento
- **Pré-processamento**: Melhoria de contraste (CLAHE) e filtro bilateral
- **Detecção**: Rede neural YOLOv3 identifica veículos
- **Análise**: Cálculo de densidade baseado na área ocupada pelos veículos

#### Saída (Output)
1. **Visual**: Janela mostrando a imagem com retângulos ao redor dos veículos detectados
2. **Textual**: Relatório em formato JSON com:
   - Nome da avenida
   - Data e hora da análise
   - Número de veículos detectados
   - Densidade do tráfego (0.0 a 1.0)
   - Status: "Tráfego Leve", "Moderado" ou "Pesado"

#### Exemplo de Saída Visual
```
┌────────────────────────────────────────────────┐
│  Avenida dos Estados - 14:30:00               │
│                                                │
│  ┌──────┐    ┌──────┐         ┌──────┐       │
│  │ Car  │    │ Car  │         │ Bus  │       │
│  └──────┘    └──────┘         └──────┘       │
│                                                │
│      ┌──────┐  ┌──────┐  ┌──────┐           │
│      │ Car  │  │Truck │  │ Car  │           │
│      └──────┘  └──────┘  └──────┘           │
│                                                │
│  Veículos detectados: 28                      │
│  Densidade: 0.169 (Tráfego Pesado)           │
└────────────────────────────────────────────────┘
```

#### Exemplo de Relatório JSON
```json
{
  "avenue": "avenida_dos_estados",
  "timestamp": "2025-11-22T14:30:00",
  "vehicle_count": 28,
  "density": 0.169,
  "status": "Heavy traffic",
  "detection_time_ms": 2340
}
```

---

## Aplicações do Sistema

### Aplicação 1: Monitoramento de Tráfego Urbano

**Objetivo**: Detectar e quantificar veículos em vias públicas para análise de densidade de tráfego.

**Cenário de Uso**: 
- Monitoramento de avenidas principais próximas à UFABC
- Identificação de horários de pico
- Suporte para planejamento urbano e gestão de tráfego

**Resultados Esperados**:
- Detecção de 15-40 veículos em horário de pico
- Densidade entre 0.08-0.20 (tráfego moderado a pesado)
- Tempo de processamento: 2-4 segundos por imagem

### Aplicação 2: Análise Comparativa de Períodos

**Objetivo**: Comparar a densidade de tráfego em diferentes horários do dia.

**Cenário de Uso**:
- Executar o sistema em 3 horários distintos (manhã, tarde, noite)
- Comparar os relatórios gerados
- Identificar padrões de tráfego

**Resultados Esperados**:
- Horário de pico (7h-9h, 17h-19h): densidade > 0.15
- Horário intermediário (10h-16h): densidade 0.08-0.15
- Horário baixo (20h-6h): densidade < 0.08

### Aplicação 3: Avaliação de Precisão do Sistema

**Objetivo**: Verificar a acurácia da detecção de veículos comparando com contagem manual.

**Cenário de Uso**:
- Observar a imagem processada
- Contar manualmente os veículos visíveis
- Comparar com o número reportado pelo sistema

**Resultados Esperados**:
- Precisão > 80% na detecção de veículos visíveis
- Alguns veículos parcialmente obstruídos podem não ser detectados
- Objetos muito pequenos (distantes) podem ser ignorados

---

## Procedimento Experimental

### Pré-requisitos

Antes de iniciar, certifique-se de que o sistema está instalado corretamente:

✅ Sistema operacional: macOS, Linux ou Windows  
✅ Dependências instaladas (OpenCV, CMake)  
✅ Arquivos do modelo YOLO baixados (yolov3.weights, yolov3.cfg)  
✅ Conexão com internet (para captura de imagens da câmera)

---

### 🔬 Experimento 1: Execução Básica do Sistema

#### Objetivo
Executar o sistema e observar o processo completo de detecção de tráfego.

#### Materiais Necessários
- Computador com o sistema instalado
- Terminal/Prompt de comando
- Papel e caneta para anotações

#### Procedimento Passo a Passo

**Passo 1: Navegue até o diretório de build**
```bash
cd security-cam-image-detector/traffic_density/build
```

**Passo 2: Execute o sistema**
```bash
cmake .. && make && ./main_exec
```

**Passo 3: Observe as mensagens no terminal**

Você verá mensagens como:
```
Avenue: avenida_dos_estados
Image Path: /path/to/resources/images/avenida_dos_estados/screenshot_2025-11-22_14-30-00.jpg
Loading YOLO model from: /path/to/yolov3.weights
YOLO model loaded successfully!
Processed Image Path: /path/to/filtered_image_avenida_dos_estados_14-30-00.png
Detected 28 vehicles...
```

**Passo 4: Observe a janela gráfica**

Uma janela será aberta mostrando:
- A imagem capturada da câmera
- Retângulos coloridos ao redor de cada veículo detectado
- Labels indicando o tipo de veículo (car, truck, bus, motorbike)

![Exemplo de Detecção](exemplo_deteccao_visual.png)
*Figura 1: Exemplo de saída visual com veículos detectados*

**Passo 5: Pressione qualquer tecla para fechar a janela**

**Passo 6: Leia o relatório no terminal**

O sistema exibirá um relatório em formato JSON:
```json
Traffic Report for avenida_dos_estados:
{
  "avenue": "avenida_dos_estados",
  "timestamp": "2025-11-22T14:30:00",
  "vehicle_count": 28,
  "density": 0.169,
  "status": "Heavy traffic"
}
```

#### Dados a Coletar

Preencha a tabela abaixo com os resultados obtidos:

| Dado                        | Valor Obtido | Observações |
|-----------------------------|--------------|-------------|
| Horário da execução         |              |             |
| Número de veículos detectados|             |             |
| Densidade calculada         |              |             |
| Status do tráfego           |              |             |
| Tempo de processamento      |              |             |

#### Questões para Reflexão

1. O número de veículos detectados condiz com o que você observa na imagem?
2. A densidade calculada reflete adequadamente a situação do tráfego?
3. Houve veículos não detectados? Por quê?

---

### 🔬 Experimento 2: Análise Comparativa de Horários

#### Objetivo
Comparar a densidade de tráfego em diferentes momentos do dia.

#### Procedimento

**Passo 1: Execute o sistema 3 vezes em horários diferentes**
- Execução 1: Período da manhã (7h-9h)
- Execução 2: Período da tarde (12h-14h)
- Execução 3: Período da noite (20h-22h)

**Passo 2: Colete os dados de cada execução**

| Horário | Veículos | Densidade | Status |
|---------|----------|-----------|--------|
| Manhã   |          |           |        |
| Tarde   |          |           |        |
| Noite   |          |           |        |

**Passo 3: Analise os resultados**
- Qual horário apresentou maior densidade?
- Os resultados condizem com sua experiência sobre o tráfego na região?
- Que fatores podem influenciar essas variações?

#### Gráfico Esperado

```
Densidade de Tráfego ao Longo do Dia
     │
0.20 │        ████
     │       █████
0.15 │      ██████     ████
     │     ███████    █████
0.10 │    ████████   ██████
     │   █████████  ███████
0.05 │  ██████████ ████████  ████
     │ ███████████████████████████
0.00 └─────────────────────────────
       6h  9h  12h  15h  18h  21h
```

---

### 🔬 Experimento 3: Verificação de Acurácia

#### Objetivo
Avaliar a precisão do sistema comparando detecção automática com contagem manual.

#### Procedimento

**Passo 1: Execute o sistema normalmente**

**Passo 2: Quando a janela gráfica aparecer, faça uma captura de tela (screenshot)**

**Passo 3: Conte manualmente os veículos na imagem**

Use a tabela abaixo:

| Tipo de Veículo | Contagem Manual | Detecção Automática | Diferença |
|-----------------|-----------------|---------------------|-----------|
| Carros          |                 |                     |           |
| Ônibus          |                 |                     |           |
| Caminhões       |                 |                     |           |
| Motos           |                 |                     |           |
| **TOTAL**       |                 |                     |           |

**Passo 4: Calcule a taxa de acurácia**

```
Acurácia = (Detecção Automática / Contagem Manual) × 100%
```

**Passo 5: Identifique discrepâncias**
- Houve veículos não detectados? (Falsos negativos)
- O sistema detectou objetos que não são veículos? (Falsos positivos)
- Anote as possíveis causas

#### Causas Comuns de Erros

- ❌ Veículos parcialmente obstruídos (atrás de outros veículos)
- ❌ Veículos muito distantes (ocupam poucos pixels)
- ❌ Condições de iluminação ruins (noite, contraluz)
- ❌ Ângulo de câmera desfavorável
- ❌ Veículos em movimento (imagem borrada)

---

### 🔬 Experimento 4: Análise de Desempenho

#### Objetivo
Medir o tempo de processamento e eficiência do sistema.

#### Procedimento

**Passo 1: Execute o sistema 5 vezes consecutivas**

**Passo 2: Anote o tempo de processamento de cada execução**

| Execução | Tempo (segundos) | Veículos Detectados |
|----------|------------------|---------------------|
| 1        |                  |                     |
| 2        |                  |                     |
| 3        |                  |                     |
| 4        |                  |                     |
| 5        |                  |                     |
| **Média**|                  |                     |

**Passo 3: Calcule a média de tempo por veículo**

```
Tempo Médio por Veículo = Tempo Total / Número de Veículos
```

#### Perguntas para Análise

1. O tempo de processamento é consistente entre as execuções?
2. O sistema é suficientemente rápido para aplicações em tempo real?
3. Que fatores podem influenciar o tempo de processamento?

---


## Enquete Subjetiva de Opinião (ESO)

### Instruções
Esta enquete visa coletar sua opinião sobre a experiência com o sistema. Seja sincero(a) em suas respostas.

---

### PARTE 1: Perguntas com Escala (1-5)

Para cada afirmação, indique seu nível de concordância:

**Escala**:
- 1 = Discordo Totalmente
- 2 = Discordo
- 3 = Neutro/Indiferente
- 4 = Concordo
- 5 = Concordo Totalmente

---

**1. O sistema foi fácil de executar e utilizar.**

(  ) 1  (  ) 2  (  ) 3  (  ) 4  (  ) 5

---

**2. As instruções do procedimento experimental eram claras e detalhadas.**

(  ) 1  (  ) 2  (  ) 3  (  ) 4  (  ) 5

---

**3. Compreendi o funcionamento técnico do sistema (captura, processamento, detecção).**

(  ) 1  (  ) 2  (  ) 3  (  ) 4  (  ) 5

---

**4. Os resultados obtidos foram coerentes com o esperado.**

(  ) 1  (  ) 2  (  ) 3  (  ) 4  (  ) 5

---

**5. A interface de saída (imagem com detecções e relatório JSON) foi clara e informativa.**

(  ) 1  (  ) 2  (  ) 3  (  ) 4  (  ) 5

---

**6. O tempo de processamento foi adequado para a aplicação.**

(  ) 1  (  ) 2  (  ) 3  (  ) 4  (  ) 5

---

**7. Considero que o sistema tem potencial para aplicações práticas reais.**

(  ) 1  (  ) 2  (  ) 3  (  ) 4  (  ) 5

---

**8. Os experimentos me ajudaram a entender conceitos de visão computacional e IA.**

(  ) 1  (  ) 2  (  ) 3  (  ) 4  (  ) 5

---

**9. Consegui identificar limitações e possíveis melhorias do sistema.**

(  ) 1  (  ) 2  (  ) 3  (  ) 4  (  ) 5

---

**10. Recomendaria este laboratório para outros estudantes.**

(  ) 1  (  ) 2  (  ) 3  (  ) 4  (  ) 5

---

### PARTE 2: Perguntas Abertas

**11. Qual foi a parte mais interessante ou surpreendente do experimento?**

```
Resposta:
_________________________________________________________________________
_________________________________________________________________________
_________________________________________________________________________
_________________________________________________________________________
```

---

**12. Qual foi a maior dificuldade que você encontrou durante a execução do sistema?**

```
Resposta:
_________________________________________________________________________
_________________________________________________________________________
_________________________________________________________________________
_________________________________________________________________________
```

---

**13. Como você avalia a precisão do sistema na detecção de veículos? Foi melhor ou pior do que você esperava?**

```
Resposta:
_________________________________________________________________________
_________________________________________________________________________
_________________________________________________________________________
_________________________________________________________________________
```

---

**14. Você conseguiu relacionar este experimento com situações do cotidiano? Dê exemplos.**

```
Resposta:
_________________________________________________________________________
_________________________________________________________________________
_________________________________________________________________________
_________________________________________________________________________
```

---

**15. Que melhorias você sugeriria para os procedimentos experimentais ou para o sistema?**

```
Resposta:
_________________________________________________________________________
_________________________________________________________________________
_________________________________________________________________________
_________________________________________________________________________
```

---



### Análise da Enquete Subjetiva de Opinião

#### Perguntas com Escala (1-5)

Para cada pergunta, calcular:
- **Média Individual**: Soma das respostas / 10
- **Média Geral**: Média de todos os usuários para cada pergunta

**Interpretação das Médias**:
- **4,5 - 5,0**: Excelente avaliação
- **3,5 - 4,4**: Boa avaliação
- **2,5 - 3,4**: Avaliação mediana (requer atenção)
- **1,5 - 2,4**: Avaliação ruim (requer melhorias)
- **1,0 - 1,4**: Avaliação crítica (revisão urgente)

#### Perguntas Abertas

Realizar análise qualitativa identificando:
1. **Temas recorrentes**: Quais pontos são mencionados por múltiplos usuários?
2. **Sugestões de melhoria**: Listar todas as sugestões para futura implementação
3. **Dificuldades comuns**: Identificar gargalos no procedimento experimental
4. **Feedback positivo**: Destacar aspectos bem-sucedidos do laboratório

---


## Relatório Final - Estrutura Sugerida

O relatório final do trabalho deverá conter:

### 1. Introdução
- Descrição do Sistema de Processamento Visual
- Objetivos do Teste de Campo

### 2. Metodologia
- Descrição dos procedimentos experimentais
- Perfil dos participantes (quantidade, formação)
- Ambiente de teste

### 3. Resultados Quantitativos
- **Tabela de notas**: Média das notas dos questionários
- **Gráfico de distribuição**: Histograma das notas obtidas
- **Estatísticas descritivas**: Média, mediana, desvio padrão

### 4. Análise da Enquete Subjetiva de Opinião
- **Gráficos de barras**: Médias das perguntas com escala (ESO1-10)
- **Análise qualitativa**: Síntese das respostas abertas
- **Principais insights**: Pontos fortes e fracos identificados

### 5. Discussão
- Interpretação dos resultados
- Comparação com expectativas iniciais
- Limitações identificadas

### 6. Conclusões e Recomendações
- Conclusões sobre a eficácia do sistema
- Recomendações de melhorias
- Próximos passos

### 7. Anexos
- Planilha completa de resultados
- Exemplos de imagens processadas
- Relatórios JSON gerados
- Respostas completas das enquetes

---

## Contato e Suporte

Em caso de dúvidas durante a execução dos experimentos:

- **GitHub**: [brMonteiro-G/security-cam-image-detector](https://github.com/brMonteiro-G/security-cam-image-detector)
- **Issues**: Abra uma issue no repositório
- **Email**: [Inserir email do responsável]

---

## Referências

1. YOLOv3: An Incremental Improvement (Redmon & Farhadi, 2018)
2. OpenCV Documentation: Deep Neural Networks module
3. Traffic Analysis using Computer Vision (IEEE Papers)
4. Experimental Laboratory Design in Computer Vision
