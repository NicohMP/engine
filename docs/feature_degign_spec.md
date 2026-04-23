# Design Note: Factors, Features, Alpha & Data Handling

## 1. Factor Definitions
- Factors are **cross-asset signals** constructed from asset-level features.
- General form:
  $$
  F_{k,t} = \mathbb{E}_{i \in G_k^{long}}[r_{i,t}] - \mathbb{E}_{i \in G_k^{short}}[r_{i,t}]
  $$
- Built via:
  - feature computation per asset
  - cross-sectional ranking / grouping
  - long-short portfolio construction
- Examples:
  - Momentum: top vs bottom quantile of past returns
  - Volatility: low-vol vs high-vol assets
  - Market: benchmark index return

---

## 2. Feature Windows
- Features rely on rolling windows:
  $$
  x_{i,t} = f(r_{i,t-1}, \dots, r_{i,t-N})
  $$
- Window sizes must be:
  - explicitly defined (e.g. 20, 60, 252)
  - consistent across assets
- Multiple windows per feature allowed (multi-scale features)
- Stored as **per-symbol rolling state** for efficiency

---

## 3. Alpha Formula
- Strategy signal per asset:
  $$
  \alpha_{i,t} = g(x_{i,t})
  $$
- Portfolio construction:
  $$
  w_{i,t} = \text{normalize}(\alpha_{i,t})
  $$
- Optional factor-neutral form:
  $$
  \alpha^{res}_{i,t} = \alpha_{i,t} - \beta_i \cdot F_t
  $$

---

## 4. Warmup Behavior
- Features require a minimum history length:
  $$
  t \geq N_{max}
  $$
- Before warmup completion:
  - no signals generated
  - no trades executed
- Engine should expose:
  - `is_warm()` flag
  - warmup length = max(feature windows, regression windows)

---

## 5. NaN Policy
- Sources:
  - insufficient history
  - missing data
- Rules:
  - feature NaN → exclude asset from ranking / signal
  - factor NaN → skip factor contribution for timestep
  - portfolio weights renormalized over valid assets
- Never forward-fill across large gaps unless explicitly configured

---

## 6. Per-Symbol State Lifecycle
Each symbol maintains independent state:

### Initialization
- allocate rolling buffers
- set state = `cold`

### Update (per timestep)
- ingest new tick
- update rolling features
- update derived signals

### Active state
- once warmup complete:
  - state = `active`
  - eligible for signals / orders

### Missing data handling
- if data absent at timestep:
  - optionally carry last price
  - do not update rolling stats unless configured

### Termination / reset
- on symbol removal:
  - clear state or archive
- allow reinitialization if symbol reappears

---