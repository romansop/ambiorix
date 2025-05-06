# Data Model Terminal User Interface

[[_TOC_]]

## Introduction

An experimental tool to make it easier to view, browse through the available data model, with the possibility to edit parameter values, create or delete instances. This tool uses ncurses to create the terminal user interface and only works over a ssh connection (no support for serial).

No extensive documentation is available (besides this readme).

## Config Options

- `objects`: (array)list of root objects that must be shown. When not defined or empty or not a list all objects will be shown if the used bus systems support discovery (amxb_list).
- `access`: (string) must be "public" or "protected". When not defined or an invalid value is specified "public" is used.

The configuration options can be specified in the file "/etc/amx/dmtui/dmtui.odl" in the config section or passed using command line options:

```bash
dmtui -o objects='["Device.","Firewall."]' -o access="protected"
```

## Usage:

### The screen layout

This terminal user interface (tui) has 3 panes:

- The left pane contains the hierarchical data model tree and will have the focus when the application has started.
- The top right pane contains the meta information of the currently selected data model tree node (can be an object or a parameter)
- The bottom right pane contains the matching objects of a search path.
- The bottom pane (stretching over the full width) is an edit pane where object paths can be entered.

The pane that has the focus will have a green border.

### Navigating between the panes

Using `<CTRL>+<arrow key>` you can selected wich pane has focus. The pane with the focus will take keyboard input.

### Navigating the data model tree

- arrow keys - browse the data model tree
  The `right` arrow key when focus is on an object will expand the object and step into it.
  The `right` arrow key when focus on a reference parameter will follow the reference.
  The `left` arrow key will collapse the current object and select the object node.
- space - collapse/expand node (in data model tree)
- enter - edit mode when possible 
  - on an object it will open a window were all writeable parameters are avialable, use up and down arrow to select, enter again to edit the value. When done use tab to select apply and press enter again
  - on parameter you immediately can edit the value, press enter to apply the new value or press esc to cancel
  - on template (multi-instance) object it will open a window were all writeable parameters and key parameters are listed, use up and down arrow to select, enter again to edit the value. When applying the values a new instance will be created.
- insert (in data model tree) - same as enter on template (multi-instance) object
- delete (in data model tree) - only works on instance object, deletes the instance

Parameters that are read-only can not be edited.

Instance can not be deleted if the parent multi-instance object has the read-only flag set.

Instances can not be created if the multi-instance object has the read-only flag set.

### Searching objects

In the bottom pane (edit box, stretching the full width of the screen), an object path can be entered. While typing the path the data model tree will follow (if the object exists ofcourse), the selected object will be set accordingly. Use TAB to complete the current node name in the edit box. When entering a '.' the data model node in the data model tree will expand.

When typing a search path the data model tree will not follow. When you entered the search path completly and press enter, all matching object paths will be displayed in the right bottom pane (search results) and focus will be set on that pane. Use the arrow keys to select the object. The data model tree will be updated and will show the selected object (if it still exists).

### Reference following

In the data model tree, when the focus is on a reference parameter (that is a parameter containing paths to other objects), the `right` arrow key can be used to jump to the referenced object. If the parameter contains multiple references (comma separated list of object references) a window will pop-up where you can select the object path you want to jump to.
