<?php

function pm_updatestatus_combined_drush_command() {
  $items = array();

  $items['pm-updatestatus-combined'] = array(
    'description' => "Get available updates from multiple domains at once.",
    'arguments' => array(
      'alias' => 'An alias (group or single) for which update status should be collected.',
    ),
    'options' => array(),
    'examples' => array(),
    'aliases' => array('upsc'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'callback' => 'drush_pm_updatestatus_combined_main',
    'outputformat' => array(
      'default' => 'table',
      'pipe-format' => 'list',
      'field-labels' => array('name' => 'Short Name', 'label' => 'Name', 'existing_version' => 'Installed Version', 'status' => 'Status', 'status_msg' => 'Message', 'candidate_version' => 'Proposed version'),
      'fields-default' => array('label', 'existing_version', 'candidate_version', 'status_msg' ),
      'fields-pipe' => array('name', 'existing_version', 'candidate_version', 'status_msg'),
      'output-data-type' => 'format-table',
    ),
  );

  return $items;
}

function drush_pm_updatestatus_combined_main() {
  $hosts = array();
  foreach (_drush_sitealias_user_specified_list() as $alias => $site) {
    if (!empty($site['root'])) {
      $host = isset($site['remote-host']) ? $site['remote-host'] : 'localhost';
      $hosts[$host][$site['root']]['sites'][] = $alias;
    }
  }

  drush_command_include('pm-updatestatus');
  drush_include_engine('release_info', 'updatexml');
  $pms = array();
  foreach ($hosts as $hostname => $host) {
    foreach ($host as $rootname => $root) {
      foreach ($root['sites'] as $site) {
        $result = drush_invoke_process(drush_sitealias_get_record('@' . $site), 'pm-list', array('--no-core', '--status=enabled'));
        foreach ($result['object'] as $name => $versions) {
          if (!isset($pms[$name][$versions['version']])) {
            $pms[$name][$versions['version']]['info'] = $versions;
            $hosts[$hostname][$rootname]['pmi'][$site][] = $name;
          }
          $pms[$name][$versions['version']]['sites'][] = $site;
        }
      }
    }
  }

  foreach ($hosts as $hostname => $host) {
    foreach ($host as $rootname => $root) {
      if (!empty($root['pmi'])) {
        foreach ($root['pmi'] as $sitename => $projects) {
          $result = drush_invoke_process(drush_sitealias_get_record('@' . $sitename), 'pm-info', array(implode(',', $projects)));
          foreach ($result['object'] as $name => $details) {
            $pms[$name][$details['version']]['info'] += $details;
            $pms[$name][$details['version']]['request'] = array(
              'name' => $name,
              'drupal_version' => $details['core'],
            );
          }
        }
      }
    }
  }

  ksort($pms);

  $result = array();
  $paths = array();
  foreach ($pms as $name => $versions) {
    foreach ($versions as $version => $detail) {
      if ($detail['info']['project'] != $name) {
        continue;
      }
      if (!empty($detail['info']['hidden'])) {
        continue;
      }
      $xml = updatexml_get_release_history_xml($detail['request']);
      if (!empty($xml)) {
        $result[$name . '_' . $version] = updatexml_get_releases_from_xml($xml, $name);
        $result[$name . '_' . $version]['installed'] = $version;
        if ($result[$name . '_' . $version]['installed'] == $result[$name . '_' . $version]['recommended']) {
          $result[$name . '_' . $version]['status'] = DRUSH_PM_REQUESTED_CURRENT;
        }
        else {
          $result[$name . '_' . $version]['status'] = DRUSH_PM_REQUESTED_UPDATE;
          $result[$name . '_' . $version]['updateable'] = TRUE;
          $result[$name . '_' . $version]['label'] = $name;
          $result[$name . '_' . $version]['existing_version'] = $version;
          $result[$name . '_' . $version]['candidate_version'] = $result[$name . '_' . $version]['recommended'];
          $result[$name . '_' . $version]['status_msg'] = implode(', ', $pms[$name][$version]['sites']);
        }
      }
      else {
        $result[$name . '_' . $version]['status'] = DRUSH_PM_REQUESTED_VERSION_NOT_FOUND;
      }
    }
  }
  return pm_project_filter($result, drush_get_option('security-only'));
}
