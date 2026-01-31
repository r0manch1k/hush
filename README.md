# Hush

![](docs/Logo-128x128.png)

**Hush** - это простой менеджер паролей, написаный на FLTK (C++).

### Разработка (MacOS)

В любом месте на компьютере соберите необходимую версию библиотеки (_FLTK 1.4.4_)

```sh
brew install cmake

curl -L -O https://github.com/fltk/fltk/releases/download/release-1.4.4/fltk-1.4.4-source.tar.gz

tar -xzf fltk-1.4.4-source.tar.gz

cd fltk-1.4.4

cmake -B build

cd build

make

sudo make install
```

В учебных целях используется простой обфускатор, который подключается в виде модуля

```
git submodule update --init --recursive
```

В папке проекта соберите сам проект

```sh
make run
```

Для форматирования кода используем `.clang-format`, который нужно скачать и настроить в своей IDE

```sh
brew install clang-format
```
