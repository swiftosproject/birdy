# Birdy

## Information

Birdy is a simple package manager intended to be used on the SwiftOS operating system. The [frontend](https://github.com/swiftosproject/birdy) is written in C++ and the [backend](https://github.com/swiftosproject/birdy-server) in Python with the use of Flask.

## Syntax and usage

### Installation

To install a package specify its name and optionally it's version, if you do not specify the version it will use the latest version.

```
birdy {--install,-i} <name> [version]
```

### Uninstallation

To uinstall a package specify its name.

```
birdy {--uninstall,-u} <name>
```

### Setting installation root

If you want to install to a different root you can use the root option followed by the mountpoint/path.

```
birdy .. {--root,-r} <root>
```

### Switching server

To switch the server from where you are downloading packages and getting package information you can use the server switch followed by the server url.

```
birdy .. --server <server>
```
