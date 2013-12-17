drush-pm-updatestatus-combined
==============================

Collect update status on modules and themes for any number of aliases at once and output in a combined table.

If you have many Drupal sites to manage and want to get an overview which projects (modules and themes) are available for update and get this all at once, then this Drush command is for you. Just define a group of aliases and then call this command and pass that alias name as a parameter.

Example:
--------

group1.aliases.drushrc.php

$aliases['site1'] = array(
  'root' => '/var/www/drupal6',
  'remote-host' => 'host1',
);
$aliases['site2'] = array(
  'root' => '/var/www/drupal7',
  'remote-host' => 'host1',
);
$aliases['site3'] = array(
  'root' => '/var/www/dev',
);

Then you can call:

drush pm-updatestatus-combined @group1

or in short:

drush upsc @group1

This will then collect update status information from all 3 sites, allthough they are use different Drupal versions and are hosted on different hosts. The drush command is optimized so that it does *not* collect status information from one and the same project more than once.
