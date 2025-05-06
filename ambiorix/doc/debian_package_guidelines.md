# Debian package creation guidelines

This is a short guide that will explain how to build a debian package for an ambiorix component.

## Debian package files

Most ambiorix components contain a debian package, which makes it easy to install them with a simple `apt install`. It is recommended that you add the debian package creation files to your component as well. Whenever someone asks you how to install the component, you can simply tell them to run `apt install <your-component>`.

For detailed information about the debian package files, refer to the official [debian policy](https://www.debian.org/doc/debian-policy/index.html). This document will only explain the minimal things you have to change when you start from the template files. For ambiorix plugins you can start from the [default plugin template](https://gitlab.com/prpl-foundation/components/ambiorix/templates/amx-default-plugin).

The debian package creation files can be found under `packages/package_debian`:

- changelog
- compat
- control
- copyright
- makefile
- rules
- triggers

You can ignore the `compat` and `triggers` file as these will be identical for all debian packages. The other files will require some changes.

### control

The `control` file contains some meta-information about your package. You will need to update the `Source` and `Package` field to reflect the name of your package. This should be the same as the `name` field in the `baf.yml` file. Note that debian package names cannot contain underscores. The `Maintainer` can be left as Softathome or can be updated to a specific user. The `Architecture` field would need to change if you wanted to build a package for a different architecture. At the moment of writing this, we don't create debian packages for different architectures, so you don't need to change this.
The `Depends` field should contain a comma-separated list of all packages your component depends on. For example if your package depends on `libamxc`, you would need to add it as a dependency here. This is needed to make sure it is installed together with your package during installation. One convenient way to find all your package dependencies is to use `${shlibs:Depends}`, which will automatically look for all the dependencies.
Keep in mind that this will only add compile time dependencies to your package dependencies. If you are writing an ambiorix plugin, you will typically need to install `amxrt` as well to be able to run the plugin. This is why it is also listed explicitly in the default plugin template. If you are writing a library, `amxrt` is of course not needed and should be removed. If your component uses [mod-dmext](https://gitlab.com/soft.at.home/sahoss/ambiorix/modules/mod-dmext), it will load this module at run time. Therefore `shlibs` won't see it as a dependency and it should also be added manually. Finally if your component depends on an older softathome package such as `sah-lib-sahtrace-dev`, you will also need to list it explicitly because these older packages don't contain the needed version information.

> ! WARNING !
> As explained above, `shlibs` will automatically look for your package dependencies, but this will only work correctly if the dependencies themselves were installed from debian packages. If this is not the case, `shlibs` will be unable to retrieve the correct package information from the dependencies. This makes sense, because if you build and install `libamxc` from source, you will have the required headers and shared object on your system (e.g. in /usr/lib/x86_64-linux-gnu/libamxc.so), which will allow you to build your package, but it gives no indication of which debian package would be needed for `libamxc.so`.
> This is usually not an issue, since our CI system is responsible for building the packages and pushing them to artifactory, but it is something important to keep in mind if you ever need to manually build and push a debian package to artifactory.

The `Description` field should contain a short description of your component. The `Section` and `Priority` fields can be ignored.

### makefile

The `makefile` is a very important file for building your package, but luckily the same recipe can always be used for building the package. This means that you don't have to change much here. The only thing that must be updated is the package description under the `AFLAGS` variable.

### changelog

The `changelog` file will contain information about the changes made to the package between different versions. It is important that the changelog file contains at least one entry, because this is needed to correctly build the debian package. The package version information is also obtained from the first line of the debian package and this version must match the latest tag of the component. If there are no tags yet, the version must be (0.0.0).

To elaborate on this, assume you create a new tag `v0.0.1` for your component and didn't update the `changelog` file, so it still mentions version (0.0.0). The package creation script will now become confused because it finds references to both version 0.0.0 and 0.0.1, hence the script will fail.

So make sure that the version on the first line corresponds with the most recent tag of your component and update the component name. If there are some relevant comments for the last tag, you can add extra bullets with a nice description and remove the line with `Initial commit`.

#### What to do for a new release

Assume you have just merged a new feature to main/master. It is now possible to generate an updated version of the changelog in the output directory by changing into the debian package directory and running the `changelog` target of the makefile. Note that you need to build a package first before this will work. The steps can be summarized as.

```bash
cd <your-component>
make clean
make
make package
cd packages/package_debian
make changelog
```

This will create an updated changelog somewhere in the output directory. You can copy this changelog to the current working directory.

```bash
cp ../../output/x86_64-linux-gnu/<your-component>/debian .
```

You will now have a changelog that looks similar to this:

```
amx-template (0.0.0-2-ged8ad12) unstable; urgency=medium

  * Issue: #6 Small bug fix in code
  * Issue: #5 Amazing new feature

 -- Johan Jacobs <johan.jacobs_ext@softathome.com>  Thu, 02 Sep 2021 14:03:35 +0000

# The rest of the file is left out
```

The script will take the commmit messages of all the commits since the last tag and list them as bullet points in the changelog. If you paid attention to your merge commit messages, you won't have to change anything here. However if you have dummy commits in the list, you should update or remove them. The version of the new package must also be updated. It is now set to 0.0.0-2-ged8ad12 or something similar and should be set to the new tag you are going to create.

When everything looks good, you can commit the changes, push them to main/master and create a new tag.

#### Letting our CI do all the work

All of the above sounds like a lot of work. It is much more convenient to let our CI system take care of these things. To do this, you can supply your merge requests with a label `tag auto`, `tag patch`, `tag minor` or `tag major`. Depending on which label you choose, the CI will automatically create a tag for your component after you merge the changes. You can generally use `tag auto` to let the CI system decide what the upstep should be. For more information refer to the posts on the Gitlab CI channel on teams.

### copyright

The `copyright` file contains the license information for your code. By default, the template in the `amx-default-plugin` contains a closed source license. Ask your project manager or team lead what the license for your component should be.

### rules

Last but not least we have the `rules` file. For plugins that run with `amxrt`, you should make sure that the file contains a rule to create a symbolic link to amxrt. In the plugin template this is done with the following line:

```bash
  dh_link usr/bin/amxrt usr/bin/amx-template
```

For libraries you must not create a symbolic link to `amxrt`, but you will need 2 other lines to create the correct symbolic links to the shared object.

```bash
	dh_link usr/lib/$(MACHINE)/<your-component>.so.$(VERSION) usr/lib/$(MACHINE)/<your-component>.so
	dh_link usr/lib/$(MACHINE)/<your-component>.so.$(VERSION) usr/lib/$(MACHINE)/<your-component>.so.$(VMAJOR)
```

Take a look at the `rules` file of [libamxc](https://gitlab.com/prpl-foundation/components/ambiorix/libraries/libamxc/-/blob/main_v1.2.2/packages/package_debian/rules) for an example.

> Note: Make sure that the shared object for your library actually contains the version information. If this information is missing, you will be adding broken symbolic links. The version information of your `.so` can be configured in the `baf.yml` file under the artifact that defines the installation of your `.so`.