<div align="center">
  <img alt="OrcaSlicer logo" src="resources/images/OrcaSlicer.png" width="120" />

# OrcaSlicer (petrpopov fork)

[![RU](https://img.shields.io/badge/README-Russian-blue?style=for-the-badge&logo=readme&logoColor=white)](README.md)
[![EN](https://img.shields.io/badge/README-English-lightgrey?style=for-the-badge&logo=readme&logoColor=white)](README.en.md)

Форк OrcaSlicer для тех, кто хочет продолжать использовать любимый слайсер с принтерами Bambu Lab через **Bambu Connect**.

</div>

## Почему этот форк

Коротко: **I don't give a shit** про конфликты команд и политические разборки. Мне нужен рабочий пайплайн: любимый слайсер + любимый принтер. Поэтому этот форк существует.

## Что это за проект

Это форк [OrcaSlicer](https://github.com/OrcaSlicer/OrcaSlicer), в котором добавлена и доработана поддержка сценария печати на Bambu Lab через Bambu Connect, а также улучшен UX в ряде мест.

Важно: основной функционал оригинальной OrcaSlicer не ломался и не переписывался «с нуля»; изменения точечные и сфокусированы на совместимости и удобстве.

## Быстрый старт печати через Bambu Connect

1. Установите Bambu Connect:
   - официальный wiki-гайд: [Bambu Connect](https://wiki.bambulab.com/en/software/bambu-connect)
   - анонс и контекст от Bambu Lab: [Updates and Third-Party Integration with Bambu Connect](https://blog.bambulab.com/updates-and-third-party-integration-with-bambu-connect/)
2. В OrcaSlicer откройте `Preferences` и включите опцию `Use Bambu Lab Connect`.
3. Перед отправкой убедитесь, что `Bambu Connect` уже запущен и видит ваш текущий принтер.
4. Подготовьте модель и нажмите `BBL Connect` в окне отправки на печать.

## macOS: установка и запуск неподписанного `.dmg`

### RU

1. Откройте (смонтируйте) `.dmg`.
2. Перетащите `OrcaSlicerPE.app` в `/Applications`.
3. Откройте `Terminal` и выполните:

```bash
xattr -dr com.apple.quarantine "/Applications/OrcaSlicerPE.app"
```

4. Запустите приложение из `/Applications`.

## Основные фичи форка

| Фича | Что даёт | Где сделано |
|---|---|---|
| Bambu Connect handoff | Отправка задания в Bambu Lab workflow через Bambu Connect из OrcaSlicer | [PR #1](https://github.com/petrpopov/OrcaSlicer/pull/1) |
| Поиск в выпадающем списке филаментов | Быстрый realtime-поиск по пресетам, включая фиксы ввода и раскладок | [PR #9](https://github.com/petrpopov/OrcaSlicer/pull/9), [PR #10](https://github.com/petrpopov/OrcaSlicer/pull/10), [PR #11](https://github.com/petrpopov/OrcaSlicer/pull/11), [PR #12](https://github.com/petrpopov/OrcaSlicer/pull/12) |
| Цветовые темы и акцентный цвет | Единый accent color для native UI и embedded web UI + полировка поведения тем | [PR #3](https://github.com/petrpopov/OrcaSlicer/pull/3), [PR #6](https://github.com/petrpopov/OrcaSlicer/pull/6), [PR #8](https://github.com/petrpopov/OrcaSlicer/pull/8) |
| Оригинальный масштаб превью-слайдеров (Bambu-like) | Возвращён привычный размер/масштаб контролов в preview | [PR #2](https://github.com/petrpopov/OrcaSlicer/pull/2) |
| Улучшения UX вокруг материалов и настроек | Prefetch AMS-списков, группировка опций форка, улучшения потока кастомных филаментов | [PR #4](https://github.com/petrpopov/OrcaSlicer/pull/4), [PR #5](https://github.com/petrpopov/OrcaSlicer/pull/5), [PR #7](https://github.com/petrpopov/OrcaSlicer/pull/7) |

## Сборка

Алгоритм сборки такой же, как у оригинальной OrcaSlicer.

- Официальная инструкция по сборке: [Orca Wiki: How to build](https://www.orcaslicer.com/wiki/How-to-build)
- Оригинальный репозиторий OrcaSlicer: [github.com/OrcaSlicer/OrcaSlicer](https://github.com/OrcaSlicer/OrcaSlicer)

## Скачать

- Релизы этого форка: [GitHub Releases](https://github.com/petrpopov/OrcaSlicer/releases)

## Лицензия и атрибуция

- Проект распространяется по лицензии **GNU Affero General Public License v3.0**. См. [LICENSE.txt](LICENSE.txt).
- Этот форк основан на [OrcaSlicer](https://github.com/OrcaSlicer/OrcaSlicer), который, в свою очередь, развивается поверх экосистемы Bambu Studio / PrusaSlicer / Slic3r.
- Все применимые лицензионные обязательства апстрима сохраняются.
