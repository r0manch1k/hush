# hush

менеджер паролей

### Разработка (MacOS)

В любом месте на компьютере соберите необходимую версию библиотеки (_FLTK 1.4.4_)

```sh
brew install cmake

curl -L -O https://github.com/fltk/fltk/releases/download/release-1.4.4/fltk-1.4 4-source.tar.gz

tar -xzf fltk-1.4.4-source.tar.gz

cd fltk-1.4.4

cmake -B build

cd build

make

sudo make install
```

В папке проекта соберите сам проект

```sh
make run
```
