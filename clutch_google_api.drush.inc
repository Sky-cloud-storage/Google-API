<?php
require __DIR__ . '/../../../libraries/google_api/autoload.php';

//if (php_sapi_name() != 'cli') {
//  throw new Exception('This application must be run on the command line.');
//}

/**
 * Returns an authorized API client.
 * @return Google_Client the authorized client object
 */
function getClient() {
  $client = new Google_Client();
  $client->setApplicationName('Google Drive API PHP Quickstart');
  $apiKey = 'AIzaSyB-H0Mv6kgBsdDbllZ7Yv6KZf_b0Ofzxws';
  $client->setDeveloperKey($apiKey);
  $client->setScopes(Google_Service_Drive::DRIVE);
  $client->setAuthConfig(__DIR__ . '/client_secret.json');
  $client->setAccessType('offline');
  $client->setPrompt('select_account consent');
  // Load previously authorized token from a file, if it exists.
  $tokenPath = __DIR__ . '/token.json';
  if (file_exists($tokenPath)) {
    $accessToken = json_decode(file_get_contents($tokenPath), TRUE);
    $client->setAccessToken($accessToken);
  }

  // If there is no previous token or it's expired.
  if ($client->isAccessTokenExpired()) {
    // Refresh the token if possible, else fetch a new one.
    if ($client->getRefreshToken()) {
      $client->fetchAccessTokenWithRefreshToken($client->getRefreshToken());
    }
    else {
      // Request authorization from the user.
      $authUrl = $client->createAuthUrl();
      printf("Open the following link in your browser:\n%s\n", $authUrl);
      print 'Enter verification code: ';
      $authCode = trim(fgets(STDIN));

      // Exchange authorization code for an access token.
      $accessToken = $client->fetchAccessTokenWithAuthCode($authCode);
      $client->setAccessToken($accessToken);

//       Check to see if there was an error.
      if (array_key_exists('error', $accessToken)) {
        throw new Exception(join(', ', $accessToken));
      }
    }
    // Save the token to a file.
    if (!file_exists(dirname($tokenPath))) {
      mkdir(dirname($tokenPath), 0700, TRUE);
    }
    file_put_contents($tokenPath, json_encode($client->getAccessToken()));
  }
  return $client;
}

/**
 * @file
 * Drush command file
 */

/**
 * Implements hook_drush_command()
 */
function clutch_google_api_drush_command() {
  $items['path-to-view-name'] = array(
    'callback' => 'drush_clutch_google_api_path_to_view_name',
    'description' => 'creates a mapping of clutch view paths to view machine names and sends the file to Google Drive',
    'aliases' => array('pv'),
  );

  return $items;
}

function drush_clutch_google_api_path_to_view_name() {
  $client = getClient();
  $service = new Google_Service_Drive($client);

  $all_views = views_get_all_views();
  $outer_array = array("Path, Machine Name\n");
  foreach ($all_views as $view) {
    if (strpos($view->name, 'lm_') === 0) {
      $inner_array = $view->display['directory']->display_options['path'] . "," . $view->name;
      array_push($outer_array, $inner_array);
    }
  }
  // drush_print_r($outer_array);
  $file = new Google_Service_Drive_DriveFile();
  $file->setName("path_to_view_machine_name" . date("Y/m/d") . ".csv");
  $result2 = $service->files->create(
    $file,
    array(
      'data' => implode("\n", $outer_array),
      'mimeType' => 'text/csv',
      'uploadType' => 'multipart',
    )
  );
  echo("Path to view export done");
}
