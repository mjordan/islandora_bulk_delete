# Islandora Bulk Deleter

A utility module that does one thing: deletes all the objects in an Islandora collection (or optionally, all issues and pages in a newspaper object). Use this tool carefully. It is dangerous.

## Requirements

* [Islandora Solr Search](https://github.com/Islandora/islandora_solr_search)
* [Islandora Paged Content](https://github.com/Islandora/islandora_paged_content) if you want to delete books, newspapers, or newspaper issues.

## Usage

The Islandora Bulk Deleter provides two drush commands. There is no graphical user interface. When you run the command, it tells you how many objects it is going to delete, and prompts you to make sure you want to go ahead. If you say 'y', it does to the resulting objects what a chainsaw does to the branches of a tree. If you say 'n', drush exits without doing anything. The chainsaw returns to idle with the safety on.

To delete objects (to purge them, to use FedoraCommons' terminology), issue a command with one of the following templates:

`drush iChainsaw --user=someuser --collection=bar:collection`
`drush iChainsaw --user=someuser --content_model=foo:contentModel --collection=bar:collection`
`drush iChainsaw --user=someuser --newspaper=some:rag`

The specified collection (or newspaper) object is not deleted. For newspaper issues and book objects, all associated page objects will also be deleted.

The `--content_model` parameter is optional, and provides the only way to limit the scope of the carnage. If you include it, only objects in the specified collection of the specified content type will be deleted. The `--user` needs to have Drupal permission to "Permanently remove objects from the repository." The values of all options other than `--user` are PIDs.

Newspapers are a special case. If you only want to delete all the issues (and their pages) within a specific newspaper, use the `--newspaper` option. If you want to delete all the newspapers (and their issues and pages) in a collection, use the regular `--collection` option.

## Maintainer

* [Mark Jordan](https://github.com/mjordan)

## License and Terms of use

* [GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
* Please review sections 15, 16, and 17 of the GPLv3 carefully before installing this module.
