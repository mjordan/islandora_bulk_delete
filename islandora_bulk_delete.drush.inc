<?php

/**
 * @file
 * Drush file for the Islandora Bulk Deleter module.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_bulk_delete_drush_command() {
  $items = array();
  $items['islandora_bulk_delete_delete'] = array(
    'aliases' => array('iChainsaw'),
    'description' => 'Delete (purge) Islandora objects, including child pages for books and newspaper issues.',
    'examples' => array(
      'drush iChainsaw --user=admin --collection=islandora:sp_basic_image_collection',
      'drush iChainsaw --user=admin --content_model=islandora:sp_basic_image --collection=islandora:sp_basic_image_collection',
    ),
    'options' => array(
      'content_model' => array(
        'description' => 'The content model of the objects to purge.',
      ),
      'content_model_field' => array(
        'description' => 'The solr field containing the content model. ' .
        'Defaults to RELS_EXT_hasModel_uri_t if not provided.',
      ),
      'namespace' => array(
        'description' => 'The namespace of the objects to purge.',
      ),
      'list' => array(
        'description' => 'List but do not purge the objects.',
      ),
      'collection' => array(
        'description' => 'The collection from which to purge the objects. ' .
        'Note: objects are purged from the repository, not just removed ' .
        'from the collection.',
      ),
      'collection_field' => array(
        'description' => 'The solr field containing collection membership. ' .
        'Defaults to RELS_EXT_isMemberOfCollection_uri_t if not provided.',
      ),
      'newspaper' => array(
        'description' => 'The newspaper from which to purge issues. ' .
        'Note: issue objects and associated pages are purged from the repository, ' .
        'not just removed from the newspaper.',
      ),
      'newspaper_issue_field' => array(
        'description' => 'The solr field containing the newspaper in which ' .
        'issues have membership. ' .
        'Defaults to RELS_EXT_isMemberOf_uri_t if not provided.',
      ),
      'pid_file' => array(
        'description' => 'Absolute path to a file containing PIDs of objects to delete, one PID per line.',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  return $items;
}

/**
 * Purges objects (including page objects).
 */
function drush_islandora_bulk_delete_delete() {
  if (!module_exists('islandora_solr')) {
    drush_set_error('IDB_SOLR_SEARCH_NOT_AVAILABLE',
      dt('Sorry, Islandora Solr Search not enabled.'));
    exit;
  }

  // If applicable, confirm the specified collection object exists and
  // is accessible; if it's not, exit with an error. Also confirm that the
  // object has the expected content model.
  if (drush_get_option('collection')) {
    if (!islandora_object_load(drush_get_option('collection'))) {
      drush_set_error('IBD_COLLECTION_NOT_FOUND',
        dt('The specified collection object (!collection) is not found or is not accessible.',
        array('!collection' => drush_get_option('collection'))));
      drupal_exit();
    }
    else {
      $collection_object = islandora_object_load(drush_get_option('collection'));
      $collection_cmodels = $collection_object->models;
      if ($collection_cmodels[0] != 'islandora:collectionCModel') {
        drush_set_error('IBD_IS_NOT_COLLECTION',
          dt('The specified collection object (!collection) does not have a content model of islandora:collectionCModel.',
          array('!collection' => drush_get_option('collection'))));
        drupal_exit();
      }
    }
    // OK, everything checks out, so we are ready to build up the Solr query
    // to get the collection's members.
    $object = $collection_object;
    $query = drush_get_option('collection_field','RELS_EXT_isMemberOfCollection_uri_t') . ':%22' .
       drush_get_option('collection') . '%22';
    if (drush_get_option('content_model')) {
      $query .= '%20AND%20' . drush_get_option('content_model_field','RELS_EXT_hasModel_uri_t'). ':%22' .
        drush_get_option('content_model') . '%22';
    }
    if (drush_get_option('namespace')) {
      $query .= '%20AND%20PID:' . drush_get_option('namespace') . '\:*&defType=edismax';
    }
  }

  // If applicable, confirm the specified newspaper object exists and
  // is accessible; if it's not, exit with an error. Also confirm that the
  // object has the expected content model.
  if (drush_get_option('newspaper')) {
    if (!islandora_object_load(drush_get_option('newspaper'))) {
      drush_set_error('IBD_NEWSPAPER_NOT_FOUND',
        dt('The specified newspaper object (!newspaper) is not found or is not accessible.',
        array('!newspaper' => drush_get_option('newspaper'))));
      drupal_exit();
    }
    else {
      $newspaper_object = islandora_object_load(drush_get_option('newspaper'));
      $newspaper_cmodels = $newspaper_object->models;
      if ($newspaper_cmodels[0] != 'islandora:newspaperCModel') {
        drush_set_error('IBD_IS_NOT_NEWSPAPER',
          dt('The specified newspaper object (!newspaper) does not have a content model of islandora:newspaperCModel.',
          array('!newspaper' => drush_get_option('newspaper'))));
        drupal_exit();
      }
    }
    // OK, everything checks out, so we are ready to build up the Solr query
    // to get the newspaper's issues.
    $object = $newspaper_object;
    $query = drush_get_option('newspaper_issue_field','RELS_EXT_isMemberOf_uri_t') . ':%22' .
      drush_get_option('newspaper') . '%22';
  }

  // If no collection or newspaper was specified, make the query use
  // namespace only.
  if (!drush_get_option('collection')  && !drush_get_option('newspaper')) {
    if (drush_get_option('namespace')) {
      $query .= 'PID:' . drush_get_option('namespace') . '\:*&defType=edismax';
    }
  }

  // Get the PIDs from theh Solr query.
  $pids = islandora_bulk_delete_query_solr($query);

  // Check to see if there is a PID file and if there is, use the PIDs
  // from it instead of from Solr.
  if (drush_get_option('pid_file')) {
    $pid_file_path = drush_get_option('pid_file');
    if (!file_exists($pid_file_path)) {
      drush_print(t("The PID file !pid_file doesn't exist", array('!pid_file' => $pid_file_path)));
      drupal_exit();
    }
    $pids = islandora_bulk_delete_read_pid_file($pid_file_path);
  }

  if (!count($pids) || !$pids) {
    drush_print('There are no matching objects to delete.');
    drupal_exit();
  }

  // Prompt the user.
  if (drush_get_option('list')) {
    $mode = 'listed only and not deleted';
  }
  else {
    $mode = 'deleted. This action is not reversible';
  }
  if (drush_confirm(dt("!num_objects objects found. Are you sure you want to continue? If you respond 'y', the objects will be !mode.",
    array('!num_objects' => count($pids), '!mode' => $mode)))) {
    if (drush_get_option('list')) {
      drush_print('OK, listing (not deleting) all those objects and any pages that are attached.');
    }
    else {
      drush_print('OK, deleting all those objects and any pages that are attached.');
    }

    // Cycle through the objects.
    foreach ($pids as $pid) {
      drupal_static_reset('islandora_get_tuque_connection');
      $object = islandora_object_load($pid);
      $cmodels = $object->models;
      // Get the current object's descendents. Assumes one content model.
      $descendent_pids = islandora_bulk_delete_get_descendents($pid, $cmodels[0]);
      // Delete the object's descendents if there are any.
      if (count($descendent_pids)) {
        foreach ($descendent_pids as $d_pid) {
          if ($d_object = islandora_object_load($d_pid)) {
            if (drush_get_option('list')) {
              drush_print(t('!pid', array('!pid' => $pid)));
            }
            else {
              if (islandora_delete_object($d_object)) {
                drush_print(t('Deleted !pid', array('!pid' => $d_pid)));
              }
              else {
                drush_print(t('Failed to delete object !pid', array('!pid' => $d_pid)));
              }
            }
          }
        }
      }
      // Then delete the parent objects from the Solr query.
      if (islandora_object_load($object->id)) {
        if (drush_get_option('list')) {
          drush_print(t('!pid', array('!pid' => $pid)));
        }
        else {
          if (islandora_delete_object($object)) {
            drush_print(t('Deleted !pid', array('!pid' => $pid)));
          }
          else {
            drush_print(t('Failed to delete object !pid', array('!pid' => $pid)));
          }
        }
      }
    }
  }
  else {
    drush_user_abort();
  }
}
