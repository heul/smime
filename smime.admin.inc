<?php

/**
 * @file
 * Admin pages for S/MIME module.
 */

/**
 * Page callback for admin/settings/smime.
 */
function smime_settings() {
  $form = array();
  $form['#attributes']['enctype'] = 'multipart/form-data';
  $form['cert'] = array('#prefix' => '<div class="container-inline">', '#suffix' => '</div>');
  $form['cert']['upload'] = array(
    '#type' => 'file',
    '#title' => t('Certificate file'),
  );
  $form['cert']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Upload'),
  );
  $form['desc'] = array(
    '#value' => '<div class="description">' . t('Certificates must be in p12 format, and should not have a passphrase.') . '</div>',
  );
  $form['table'] = array(
    '#value' => smime_list_keys(),
  );
  return $form;
}

/**
 * Submit callback for admin/settings/smime.
 */
function smime_settings_submit(&$form, &$form_state) {
  if (isset($_FILES['files']['size']['upload']) && $_FILES['files']['size']['upload'] > 0) {
    $cert = file_get_contents($_FILES['files']['tmp_name']['upload']);
    if (!smime_add($cert)) {
      form_set_error('upload', t("The certificate file failed to import."));
    }
    else {
      drupal_set_message(t('The certificate file has been added.'));
    }
  }
}


/**
 * Form callback for admin/settings/smime/delete/%.
 */
function smime_delete_confirm($form_state, $cert) {
  return confirm_form(
    array(),
    t('Are you sure you want to delete the certificate for %mail?', array('%mail' => $cert['email'])),
    'admin/settings/smime',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Page callback for admin/settings/smime/view/%.
 */
function smime_view_page($cert) {
  drupal_set_title(t('Certificate details for %email', array('%email' => $cert['email'])));
  $certs = array();
  if (openssl_pkcs12_read($cert['cert'], $certs, '')) {
    $descriptorspec = array(
       0 => array("pipe", "r"),  // stdin is a pipe that the child will read from
       1 => array("pipe", "w"),  // stdout is a pipe that the child will write to
    );
    $process = proc_open('openssl x509 -text', $descriptorspec, $pipes);
    if (is_resource($process)) {
      fwrite($pipes[0], $certs['cert']);
      fclose($pipes[0]);
      $decoded_cert = stream_get_contents($pipes[1]);
      fclose($pipes[1]);
      $return_value = proc_close($process);
      if ($return_value !== 0) {
        // Fall back to PHP's openssl implementation.
        $parsed_cert = openssl_x509_parse($certs['cert']);
        return '<pre style="width:100%; overflow:auto;">' . check_plain(var_export($parsed_cert, TRUE)) . '</pre>';
      }
      return '<pre style="width:100%; overflow:auto;">' . check_plain($decoded_cert) . '</pre>';
    }
  }
}

/**
 * Page callback for admin/settings/smime/download.
 */
function smime_download_page($cert) {
  drupal_set_header('Content-type: application/x-pkcs12');
  drupal_set_header('Content-Disposition: attachment; filename="'. $cert['email'] .'.p12'.'"');
  print $cert['cert'];
  exit();
}

/**
 * Submit callback for admin/settings/smime/delete/%.
 */
function smime_delete_confirm_submit($form, &$form_state) {
  $cert = menu_get_object('smime', 4);
  smime_delete($cert['id']);
  drupal_set_message(t('The certificate for %email has been deleted', array('%email' => $cert['email'])));
  $form_state['redirect'] = 'admin/settings/smime';
}


/**
 * Generates an HTML table of all stored certificates.
 */
function smime_list_keys() {
  $result = db_query("SELECT id, email, expires, issuer FROM {smime} ORDER BY email");
  $keys = array();
  while ($row = db_fetch_array($result)) {
    $row['expires'] = format_date($row['expires']);
    $row[] = l(t('View'), "admin/settings/smime/view/$row[id]") . ' ' .
      l(t('Download'), "admin/settings/smime/download/$row[id]") . ' ' .
      l(t('Delete'), "admin/settings/smime/delete/$row[id]");
    unset($row['id']);
    $keys[] = $row;
  }
  if (!empty($keys)) {
    $header = array(t('E-mail'), t('Expires'), t('Issued by'), t('Operations'));
    return filter_xss_admin(theme('table', $header, $keys));
  }
}
