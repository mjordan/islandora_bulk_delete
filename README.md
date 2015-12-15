# Islandora Bulk Deleter

A utility module that does one thing: deletes all the objects in an Islandora collection (or optionally, all issues and pages in a newspaper object). Use this tool carefully. It is dangerous.

## Requirements

* [Islandora Solr Search](https://github.com/Islandora/islandora_solr_search)
* [Islandora Paged Content](https://github.com/Islandora/islandora_paged_content) if you want to delete books, newspapers, or newspaper issues.

## Installation

`git clone https://github.com/mjordan/islandora_bulk_delete.git`
`drush en -y islandora_bulk_delete`

## Usage

There is no graphical user interface for this module. It only provides one drush command, `drush islandora_bulk_delete_delete`, or `drush iChainsaw` for short.

When you run the command, it tells you how many objects it is going to delete, and prompts you to make sure you want to go ahead. If you say 'y', it does to the resulting objects what a chainsaw does to the branches of a tree. If you say 'n', drush exits without doing anything.

However, there is a safety switch on this chainsaw. If you include the `--list` option, the objects are only listed and not deleted. The `--list` option does not take a value, unlike the other options described below.

To delete objects (to purge them, to use FedoraCommons' terminology), issue a command with one of the following templates:

* `drush iChainsaw --user=admin --collection=bar:collection`
* `drush iChainsaw --user=admin --collection=bar:collection --content_model=foo:contentModel --namespace=baz --list`
* `drush iChainsaw --user=admin --newspaper=sleazy:tabloid`

The `--content_model` and `--namespace` parameters are optional, and provide two ways to limit the set of objects within a collection to be deleted. If you include them, only objects in the specified collection of the specified content type and/or having the specified namespace will be deleted. The `--user` needs to have Drupal permission to "Permanently remove objects from the repository." The values of the `--collection` and `--content_model` options are PIDs.

The specified collection (or newspaper) object is not deleted. For newspaper issues and book objects, all associated page objects are deleted. Please note that an object is deleted even if it is in another collection. It is not simply removed from the current collection.

Newspapers are a special case:

* If you only want to delete all the issues (and their pages) within a specific newspaper, use the `--newspaper` option.
* If you want to delete all the newspapers (and their issues and pages) in a collection, use the regular `--collection` option.

## Maintainer

* [Mark Jordan](https://github.com/mjordan)

## License and Terms of use

* [GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
* Please review sections 15, 16, and 17 of the GPLv3 carefully before installing this module.
