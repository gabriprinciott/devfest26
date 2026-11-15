# Soluzioni Code Challenge — DevFest Catania 2026

## Indice

1. [Sfida 1: `pause.cpp` (C++)](#2-sfida-1-pausecpp-c)
2. [Sfida 2: `squeeze.cpp` (C++)](#3-sfida-2-squeezecpp-c)
3. [Sfida 3: `amnesia.py` (Python)](#4-sfida-3-amnesiapy-python)
4. [Sfida 4: `paperclips/paperclips.go` (Go)](#5-sfida-4-paperclipspaperclipsgo-go)

---

## 1. Sfida 1: `pause.cpp` (C++)

### Specifica
Dati una durata totale `duration` e `count` prenotazioni con intervallo `[start, end)`, trovare tutti i buchi temporali liberi continui in ordine cronologico. Se l'agenda è interamente occupata, stampare `-`.

### Bug nel codice originale
1. **Nessun ritaglio degli intervalli (clipping)**: Prenotazioni con `start < 0` o `end > duration` non venivano ritagliate entro i limiti `[0, duration)`.
2. **Intervalli vuoti non gestiti**: Intervalli non validi o con `start >= end` non venivano scartati.
3. **Calcolo errato del cursore su sovrapposizioni/nidificazioni**: Il codice eseguiva `cursor = end` in sequenza. Se un intervallo era contenuto in un altro (es. `[1, 10]` seguito da `[2, 5]`), il cursore retrocedeva a 5, provocando la generazione di falsi buchi temporali o buchi sovrapposti.

### Risoluzione
1. Ritaglio di ogni intervallo con `start = std::max(0LL, start)` e `end = std::min(duration, end)`.
2. Filtro per mantenere solo intervalli con `start < end`.
3. Ordinamento e **merge** classico degli intervalli sovrapposti o adiacenti (`merged.back().second = std::max(merged.back().second, end)`).
4. Iterazione sugli intervalli uniti per raccogliere gli spazi vuoti tra `cursor` e l'inizio del prossimo blocco occupato.

```cpp
std::vector<Interval> free_slots(long long duration, std::vector<Interval> busy) {
    std::vector<Interval> clipped;
    clipped.reserve(busy.size());
    for (auto [start, end] : busy) {
        start = std::max(0LL, start);
        end = std::min(duration, end);
        if (start < end) {
            clipped.emplace_back(start, end);
        }
    }
    std::sort(clipped.begin(), clipped.end());

    std::vector<Interval> merged;
    for (auto [start, end] : clipped) {
        if (merged.empty() || start > merged.back().second) {
            merged.emplace_back(start, end);
        } else {
            merged.back().second = std::max(merged.back().second, end);
        }
    }

    std::vector<Interval> gaps;
    long long cursor = 0;
    for (auto [start, end] : merged) {
        if (start > cursor) gaps.emplace_back(cursor, start);
        cursor = end;
    }
    if (cursor < duration) gaps.emplace_back(cursor, duration);
    return gaps;
}
```

---

## 2. Sfida 2: `squeeze.cpp` (C++)

### Specifica
Individuare la sottosequenza contigua più lunga di eventi con al massimo `limit` categorie distinte e costo complessivo $\le \text{budget}$.
- A parità di lunghezza massima: scegliere quella con costo totale minore.
- A parità di costo: scegliere quella con indice di inizio minore.
- Se non esiste alcuna sequenza valida non vuota: restituire `0 0`.

### Bug nel codice originale
1. **La mappa delle categorie non si riduceva**: Eseguendo `--counts[old_category]`, la chiave rimaneva presente in `std::unordered_map` anche a conteggio 0, quindi `counts.size()` non decresceva mai.
2. **Contrazione a singolo step con `if` anziché `while`**: Se entrava un evento con costo superiore al budget o nuova categoria, la finestra si muoveva solo di un indice senza garantire il rispetto dei vincoli.
3. **Criterio di pareggio invertito**: Usando `(length == best_len && total <= best_cost)`, il `<=` sovrascriveva una finestra precedente valida con una successiva avente lo stesso costo, violando la regola *"scegli quella che inizia prima"*.
4. **Mancata gestione di `limit <= 0`**: In caso di limite nullo nessuna sequenza non vuota è ammissibile.

### Risoluzione
1. Gestione di una variabile `distinct` aggiornata solo quando il conteggio di una categoria passa da 0 a 1 o da 1 a 0.
2. Ciclo `while` per contrarre il puntatore `left` finché `distinct > limit || total > budget`.
3. Condizione di pareggio stretta `total < best_cost`, garantendo così la preferenza per l'inizio cronologicamente precedente in caso di parità di costo.
4. Ritorno immediato di `{0, 0}` se `limit <= 0` o `budget < 0`.

```cpp
std::pair<int, int> longest_window(const std::vector<Event>& events, int limit, long long budget) {
    if (limit <= 0 || budget < 0) {
        return {0, 0};
    }

    int n = static_cast<int>(events.size());
    std::unordered_map<int, int> counts;
    int distinct = 0;
    long long total = 0;
    int left = 0;

    int best_len = 0;
    long long best_cost = 0;
    std::pair<int, int> best{0, 0};

    for (int right = 0; right < n; ++right) {
        const auto [category, cost] = events[right];
        if (counts[category] == 0) {
            ++distinct;
        }
        ++counts[category];
        total += cost;

        while (left <= right && (distinct > limit || total > budget)) {
            const auto [old_category, old_cost] = events[left];
            --counts[old_category];
            if (counts[old_category] == 0) {
                --distinct;
            }
            total -= old_cost;
            ++left;
        }

        if (left <= right) {
            const int length = right - left + 1;
            if (length > best_len) {
                best_len = length;
                best_cost = total;
                best = {left, right + 1};
            } else if (length == best_len) {
                if (total < best_cost) {
                    best_cost = total;
                    best = {left, right + 1};
                }
            }
        }
    }
    return best;
}
```

---

## 3. Sfida 3: `amnesia.py` (Python)

### Specifica
Implementare una cache LRU (Least Recently Used) con capienza `capacity` che riceve una sequenza di operazioni `put` e `get`.
- Ogni `get` riuscito e ogni `put` (inserimento o aggiornamento) rendono la chiave la più recente.
- L'aggiornamento di un elemento esistente non deve mai causare l'evizione di altre chiavi.
- Se la chiave è assente su `get`, restituire `null` senza modificare l'ordine d'uso.
- Con `capacity == 0`, non viene conservato alcun dato.

### Bug nel codice originale
1. **`get` non aggiornava la priorità**: Mancava `cache.move_to_end(key)`, quindi le chiavi lette di frequente rischiavano di essere rimosse come se fossero le meno usate.
2. **`put` eliminava chiavi negli aggiornamenti**: Se la cache era piena e veniva aggiornato il valore di una chiave *già presente*, veniva comunque chiamato `cache.popitem(last=False)`, espellendo erroneamente un'altra chiave.

### Risoluzione
1. Su operazione `get`: se la chiave è presente in `cache`, si invoca `cache.move_to_end(key)` e si appende il valore; altrimenti si appende `None`.
2. Su operazione `put`:
   - Se `capacity <= 0`: l'operazione non fa nulla.
   - Se `key in cache`: si aggiorna il valore e si invoca `cache.move_to_end(key)` senza espulsioni.
   - Se `key not in cache`: se `len(cache) >= capacity` si espelle l'elemento meno recente con `cache.popitem(last=False)`, poi si inserisce la nuova coppia chiave-valore.

```python
def replay(capacity, operations):
    cache = OrderedDict()
    results = []
    for op in operations:
        kind = op[0]
        key = op[1]
        if kind == "get":
            if key in cache:
                cache.move_to_end(key)
                results.append(cache[key])
            else:
                results.append(None)
        elif kind == "put":
            if capacity > 0:
                value = op[2]
                if key in cache:
                    cache[key] = value
                    cache.move_to_end(key)
                else:
                    if len(cache) >= capacity:
                        cache.popitem(last=False)
                    cache[key] = value
    return results
```

---

## 4. Sfida 4: `paperclips/paperclips.go` (Go)

### Specifica
Simulazione della produzione di graffette. Dati denaro iniziale `money`, filo iniziale `wire` e una lista di operazioni:
- `buy`: spende 1 unità di denaro per acquistare 1 unità di filo.
- `make`: consuma 1 unità di filo per produrre 1 graffetta.
- `sell`: vende 1 graffetta per 2 unità di denaro, togliendola dall'inventario.
- Se le risorse per completare un'azione non sono sufficienti o la quantità è zero/negativa, l'azione viene interamente ignorata.

### Bug nel codice originale
1. Nel blocco `case "make"`: veniva incrementato `clips` ma **non veniva decrementato `wire`** (`wire -= quantity`).
2. Nel blocco `case "sell"`: veniva aumentato `money` ma **non venivano scalate le graffette dall'inventario** (`clips -= quantity`).

### Risoluzione
Aggiunta delle sottrazioni corrispondenti alle risorse consumate e controllo per ignorare le operazioni con quantità non positiva (`quantity <= 0`):

```go
func simulate(money, wire int64, operations []Operation) map[string]int64 {
	clips := int64(0)
	for _, operation := range operations {
		quantity := operation.Quantity
		if quantity <= 0 {
			continue
		}
		switch operation.Kind {
		case "buy":
			if money >= quantity {
				money -= quantity
				wire += quantity
			}
		case "make":
			if wire >= quantity {
				wire -= quantity
				clips += quantity
			}
		case "sell":
			if clips >= quantity {
				clips -= quantity
				money += 2 * quantity
			}
		}
	}
	return map[string]int64{"money": money, "wire": wire, "clips": clips}
}
```