# Контракты Body Decoder

Layer 4 в `iohttpparser` отвечает только за framing тела сообщения. Этот слой не владеет транспортными буферами, не делает скрытых аллокаций и не декодирует application-level content codings.

## Chunked Decoder

- `ihtp_decode_chunked()` работает инкрементально. Один и тот же `ihtp_chunked_decoder_t` нужно переиспользовать между вызовами по мере прихода новых байтов.
- Декодер переписывает буфер вызывающей стороны in-place и обновляет `*bufsz` до числа payload-байтов, оставшихся в текущем срезе.
- `total_decoded` считает только payload bytes.
- При завершении неотрицательное возвращаемое значение означает успех и равно числу undecoded trailing bytes.
- Если `consume_trailer == false`, терминальный `CRLF` после zero chunk и все последующие байты остаются в trailing data.
- Если `consume_trailer == true`, trailer lines потребляются до завершающей пустой строки, после чего возвращается число trailing bytes.
- trailing bytes остаются в том же caller-owned buffer сразу после decoded payload prefix
- decoder не аллоцирует память и не забирает ownership ни у payload, ни у trailer bytes

## Fixed-Length Decoder

- `ihtp_fixed_decoder_init()` задаёт ожидаемую длину payload.
- `ihtp_decode_fixed()` делает только accounting: без копирования, без переписывания framing, без скрытых аллокаций.
- `remaining` и `total_decoded` меняются только в сторону завершения.
- Передача больше байтов, чем осталось в `remaining`, считается ошибкой.
