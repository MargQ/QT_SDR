
### Параметры

- **timestamp_every**: `1920`
- **gain_control_mode**: `slow_attack`
- **Sample rate TX/RX**: `1e6`
- **setFrequency TX/RX**: `800e6`

---

### Полученные данные

- **Путь к данным**: `./data`
  - `txdata*.pcm`: чистый QPSK
  - `txdata_bark*.pcm`: QPSK + код Баркера

---

### Данные по чистому QPSK

- **Битовая последовательность**:
```
[0 1 0 1 0 1 0 0 0 1 1 0 1 0 0 0 0 1 1 0 1 0 0 1 0 1 1 1 0 0 1 1 0 0 1 0 0
 0 0 0 0 1 1 0 1 0 0 1 0 1 1 1 0 0 1 1 0 0 1 0 0 0 0 0 0 1 1 0 0 0 0 1 0 0
 1 0 0 0 0 0 0 1 1 1 0 1 0 0 0 1 1 0 0 1 0 1 0 1 1 1 1 0 0 0 0 1 1 1 0 1 0
 0 0 0 1 0 0 0 0 0 0 0 1 1 1 1 1 1 0 0 1 0 0 0 0 0 0 1 0 1 1 0 0 1 0 1 1 0
 0 1 0 1 0 1 1 1 0 0 1 1 0 0 1 0 0 0 0 0 0 0 1 0 0 0 0 1]
```
- **Длина данных**: `88.0`
- **Длина последовательности**: `88`
- **Комплексные данные**:
```
[-1.+1.j -1.+1.j -1.+1.j -1.-1.j -1.+1.j  1.-1.j  1.-1.j -1.-1.j -1.+1.j
  1.-1.j  1.-1.j -1.+1.j -1.+1.j  1.+1.j -1.-1.j  1.+1.j -1.-1.j  1.-1.j
 -1.-1.j -1.-1.j -1.+1.j  1.-1.j  1.-1.j -1.+1.j -1.+1.j  1.+1.j -1.-1.j
  1.+1.j -1.-1.j  1.-1.j -1.-1.j -1.-1.j -1.+1.j  1.-1.j -1.-1.j -1.+1.j
 -1.-1.j  1.-1.j -1.-1.j -1.-1.j -1.+1.j  1.+1.j -1.+1.j -1.-1.j -1.+1.j
  1.-1.j -1.+1.j -1.+1.j -1.+1.j  1.+1.j  1.-1.j -1.-1.j -1.+1.j  1.+1.j
 -1.+1.j -1.-1.j -1.-1.j  1.-1.j -1.-1.j -1.-1.j -1.-1.j  1.+1.j  1.+1.j
  1.+1.j -1.-1.j  1.-1.j -1.-1.j -1.-1.j -1.+1.j -1.+1.j  1.-1.j -1.+1.j
 -1.+1.j  1.-1.j -1.+1.j -1.+1.j -1.+1.j  1.+1.j -1.-1.j  1.+1.j -1.-1.j
  1.-1.j -1.-1.j -1.-1.j -1.-1.j  1.-1.j -1.-1.j -1.+1.j]
```

---

### QPSK + код Баркера

- **Битовая последовательность**:
```
[0 1 0 1 0 1 0 0 0 1 1 0 1 0 0 0 0 1 1 0 1 0 0 1 0 1 1 1 0 0 1 1 0 0 1 0 0
 0 0 0 0 1 1 0 1 0 0 1 0 1 1 1 0 0 1 1 0 0 1 0 0 0 0 0 0 1 1 0 0 0 0 1 0 0
 1 0 0 0 0 0 0 1 1 1 0 1 0 0 0 1 1 0 0 1 0 1 0 1 1 1 1 0 0 0 0 1 1 1 0 1 0
 0 0 0 1 0 0 0 0 0 0 0 1 1 1 1 1 1 0 0 1 0 0 0 0 0 0 1 0 1 1 0 0 1 0 1 1 0
 0 1 0 1 0 1 1 1 0 0 1 1 0 0 1 0 0 0 0 0 0 0 1 0 0 0 0 1]
```
- **Длина данных**: `88.0`
- **Длина последовательности**: `101`
- **Комплексные данные**:
```
[ 1.+0.j  1.+0.j  1.+0.j  1.+0.j  1.+0.j -1.+0.j -1.+0.j  1.+0.j  1.+0.j
 -1.+0.j  1.+0.j -1.+0.j  1.+0.j -1.+1.j -1.+1.j -1.+1.j -1.-1.j -1.+1.j
  1.-1.j  1.-1.j -1.-1.j -1.+1.j  1.-1.j  1.-1.j -1.+1.j -1.+1.j  1.+1.j
 -1.-1.j  1.+1.j -1.-1.j  1.-1.j -1.-1.j -1.-1.j -1.+1.j  1.-1.j  1.-1.j
 -1.+1.j -1.+1.j  1.+1.j -1.-1.j  1.+1.j -1.-1.j  1.-1.j -1.-1.j -1.-1.j
 -1.+1.j  1.-1.j -1.-1.j -1.+1.j -1.-1.j  1.-1.j -1.-1.j -1.-1.j -1.+1.j
  1.+1.j -1.+1.j -1.-1.j -1.+1.j  1.-1.j -1.+1.j -1.+1.j -1.+1.j  1.+1.j
  1.-1.j -1.-1.j -1.+1.j  1.+1.j -1.+1.j -1.-1.j -1.-1.j  1.-1.j -1.-1.j
 -1.-1.j -1.-1.j  1.+1.j  1.+1.j  1.+1.j -1.-1.j  1.-1.j -1.-1.j -1.-1.j
 -1.+1.j -1.+1.j  1.-1.j -1.+1.j -1.+1.j  1.-1.j -1.+1.j -1.+1.j -1.+1.j
  1.+1.j -1.-1.j  1.+1.j -1.-1.j  1.-1.j -1.-1.j -1.-1.j -1.-1.j  1.-1.j
 -1.-1.j -1.+1.j]
```

---

### Код Баркера

```python
def barker_sequence():
    return np.array([1, 1, 1, 1, 1, -1, -1, 1, 1, -1, 1, -1, 1], dtype=np.int8)