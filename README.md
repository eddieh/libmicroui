# MicroUI (µUI)

A *tiny*, portable, immediate-mode UI library written in ANSI C

## Features

* Tiny: around `1100 sloc` of ANSI C
* Works within a fixed-sized memory region: no additional memory is
  allocated
* Built-in controls: window, scrollable panel, button, slider,
  textbox, label, checkbox, wordwrapped text
* Designed to allow the user to easily add custom controls
* Simple layout system

## Example

```c
if (mu_begin_window(ctx, "MuWindow", mu_rect(10, 10, 140, 86))) {
    mu_layout_row(ctx, 2, (int[]) { 60, -1 }, 0);

    mu_label(ctx, "First:");
    if (mu_button(ctx, "Button1")) {
        printf("Button1 pressed\n");
    }

    mu_label(ctx, "Second:");
    if (mu_button(ctx, "Button2")) {
        mu_open_popup(ctx, "My Popup");
    }

    if (mu_begin_popup(ctx, "My Popup")) {
        mu_label(ctx, "Hello world!");
        mu_end_popup(ctx);
    }

    mu_end_window(ctx);
}
```

## Build & Install

Install SDL2, then run make.

    $ make
    $ make install


## Advanced Build & Install

Verbose build

    $ make V=1

View man pages

    $ man -M . libmicroui
    $ man -M . mudemo

Install to a different PREFIX (default is $HOME)

    $ make PREFIX=/usr/local
    $ make PREFIX=/usr/local install


## Notes

- I am not the original author. This is a fork of
  (https://github.com/rxi/microui). As per the upstream project "pull
  requests adding additional features will likely not be merged," so I
  seriously doubt they'd be interested in what I'm about to do to this
  project.


## License

This library is free software; you can redistribute it and/or modify
it under the terms of the MIT license. See [LICENSE](LICENSE) for
details.
