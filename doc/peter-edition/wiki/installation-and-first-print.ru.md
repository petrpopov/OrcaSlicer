# OrcaSlicer PE: установка и первая печать

Эта инструкция рассчитана на пользователя, который впервые ставит форк `OrcaSlicer PE` и хочет печатать на принтере Bambu Lab через `Bambu Connect`.

## Что понадобится

1. `OrcaSlicer PE` из раздела [Releases](https://github.com/petrpopov/OrcaSlicer/releases).
2. `Bambu Connect` от Bambu Lab.
   Скачать можно:
   - с wiki Bambu Lab: [Bambu Connect](https://wiki.bambulab.com/en/software/bambu-connect)
   - из анонса Bambu Lab с описанием сценария: [Updates and Third-Party Integration with Bambu Connect](https://blog.bambulab.com/updates-and-third-party-integration-with-bambu-connect/)
3. Доступ к вашему принтеру в той же сети или через поддерживаемый Bambu Connect сценарий.

## Установка OrcaSlicer PE

### Windows

1. Скачайте архив или установочный пакет из релиза.
2. Распакуйте или установите приложение.
3. Запустите `OrcaSlicerPE`.

### Linux

1. Скачайте архив или пакет из релиза.
2. Распакуйте его в удобное место.
3. При необходимости сделайте бинарник исполняемым.
4. Запустите OrcaSlicer PE.

### macOS

1. Скачайте `.dmg` из релиза.
2. Откройте образ и перетащите `OrcaSlicerPE.app` в `/Applications`.
3. Так как сборка не подписана Apple, откройте `Terminal` и выполните:

```bash
xattr -dr com.apple.quarantine "/Applications/OrcaSlicerPE.app"
```

4. После этого запустите приложение из `/Applications`.

## Установка Bambu Connect

1. Скачайте и установите `Bambu Connect` по одной из официальных ссылок выше.
2. Запустите `Bambu Connect`.
3. Убедитесь, что он авторизован и видит ваш принтер.
4. Перед отправкой из OrcaSlicer PE держите `Bambu Connect` запущенным.

Важно: `OrcaSlicer PE` не заменяет `Bambu Connect`, а передаёт в него подготовленное задание на печать.

## Первичная настройка в OrcaSlicer PE

1. Запустите `OrcaSlicer PE`.
2. Откройте `Preferences`.
3. Включите опцию `Use Bambu Lab Connect`.
4. Выберите принтер, профиль сопла, материал и остальные параметры печати.

В форке эта интеграция дополняется отдельными UX-улучшениями: предзагрузка AMS-списков, более удобный поиск материалов, группировка собственных настроек форка и доработанный сценарий handoff в Bambu Connect.

## Как отправить модель на печать

1. Откройте или создайте проект.
2. Добавьте модель.
3. Выберите профиль принтера и материала.
4. Нажмите `Slice plate`.
5. Откройте диалог отправки задания.
6. Нажмите `BBL Connect`.
7. OrcaSlicer PE подготовит проект и передаст его в `Bambu Connect`.
8. Подтвердите отправку уже в `Bambu Connect`, если это требуется в вашем сценарии.

## Что изменено в OrcaSlicer PE

С первого выпуска форк добавил и развил следующие вещи:

- интеграцию с `Bambu Connect` прямо из окна отправки на печать;
- использование имени проекта в экспортируемом задании вместо технического имени файла;
- автоматическое закрытие диалога отправки после успешного запуска `Bambu Connect`;
- поиск по выпадающему списку материалов, включая исправления backspace, раскладок и сброса фильтра;
- предзагрузку AMS-списка для более удобного выбора материалов;
- отдельную группу настроек форка в `Preferences`;
- улучшенный сценарий управления кастомными филаментами;
- единый accent color для native UI и embedded web UI;
- возврат привычного Bambu-подобного масштаба preview-слайдеров;
- брендинг Peter's Edition в about/splash;
- ряд доработок стабильности, совместимости с OpenGL core/ES, обновлением через GitHub Releases и синхронизацией с upstream OrcaSlicer.

## Если печать не отправляется

- Проверьте, что `Bambu Connect` установлен и уже запущен.
- Проверьте, что в `Preferences` включён `Use Bambu Lab Connect`.
- Убедитесь, что выбран актуальный принтер и корректный проект.
- На macOS проверьте, что с приложения снят quarantine-атрибут.

## Полезные ссылки

- Релизы: [GitHub Releases](https://github.com/petrpopov/OrcaSlicer/releases)
- Текст релиза `v2.4.1`: [release notes](https://github.com/petrpopov/OrcaSlicer/blob/main/doc/peter-edition/releases/v2.4.1.md)
- Апстрим-проект: [OrcaSlicer](https://github.com/OrcaSlicer/OrcaSlicer)
