<?php

/**
 * Page callback: Rb Weather locations settings
 * 
 * @see rb_weather_menu()
 * @see ajax_display_found_locations($form, $form_state)
 */
function rb_weather_locations_config_form($form, &$form_state) {
	$form_state['storage']['locations'] = isset($form_state['storage']['locations'])
			? $form_state['storage']['locations']
			: array();
	
	$form['#prefix'] = '<div id="add-location-form">';
	$form['#suffix'] = '</div>';
			
	$form['locations_config'] = array(
			'#type' => 'fieldset',
			'#title' => t('Set locations for weather display'),
			'#collapsible' => FALSE,
	);
	$form['locations_config']['location'] = array(
			'#type' => 'textfield',
			'#title' => t('Weather station name'),
			'#description' => t('You will obtain the location code at wunderground.com'),
			'#required' => TRUE,
	);
	$form['submit'] = array(
			'#type' => 'submit',
			'#value' => t('Search for a weather station'),
			'#ajax' => array(
					'callback' => 'ajax_rb_weather_locations_config_form_callback',
					'wrapper' => 'add-location-form',
					'method' => 'replace',
					'effect' => 'fade',
			),
	);
	
	if ( ! empty($form_state['storage']['locations'])) {
		unset($form['locations_config']['location']['#required']);
		
		$form['locations_config']['locations'] = array(
				'#type' => 'select',
				'#title' => t('Choose a weather station from these options'),
				'#options' => array(t('------ Select a location -----')),
				'#prefix' => '<div id="locations">',
				'#suffix' => '</div>',
		);
		
		$form['locations_config']['locations']['#options'] += $form_state['storage']['locations'];
		
		$form['submit']['#value'] = t('Add selected weather station');
	}
	
	return $form;
}

/**
 * Implements hook_form_validate().
 * 
 * @param unknown $form
 * @param unknown $form_state
 */
function rb_weather_locations_config_form_validate($form, &$form_state) {
	$location = $form_state['values']['location'];
	
	if ( ! isset($form_state['values']['locations'])) {
		$form_state['values']['locations'] = 0;
	}
	
	if (empty($location)
			&& is_numeric($form_state['values']['locations'])
			&& !isset($form['locations_config']['locations']))  {
		form_set_error('location', t('You have to select a weather location'));
		return;
	}
	else {
		$parsed_locations = json_decode(file_get_contents('http://autocomplete.wunderground.com/aq?query=' . urlencode($location)));
		
		if (empty($parsed_locations->RESULTS)) {
			drupal_set_message(t('No results match your search query'), 'error');
			$form_state['storage']['locations'] = array();
		}
		else {
			foreach ($parsed_locations->RESULTS as $possible_location) {
				$form_state['storage']['locations'] += array($possible_location->l => $possible_location->name);
			}
		}
	}
}

/**
 * Implements hook_form_submit().
 * 
 * @param unknown $form
 * @param unknown $form_state
 */
function rb_weather_locations_config_form_submit($form, &$form_state) {
	if (!is_numeric($form_state['values']['locations'])) {
		$wlid = db_insert('rb_weather_locations')
		->fields(array(
				'weather_location_identifier' => $form_state['values']['locations'],
				'location_name' => $form_state['storage']['locations'][$form_state['values']['locations']],
		))
		->execute();
		
		drupal_set_message(t('A new weather location has successfully been added'));
		
		$form_state['storage']['locations'] = array();
	}
	
	$form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback reloading the form.
 * 
 * @param $form
 * @param $form_state
 */
function ajax_rb_weather_locations_config_form_callback($form, &$form_state) {
	return $form;
}

/**
 * Implements hook_form().
 * 
 * @param $form
 * @param $form_state
 */
function rb_weather_saved_locations_form($form, &$form_state) {
	$form['#prefix'] = '<div id="saved-locations-form">';
	$form['#suffix'] = '</div>';
	
	$locations = get_saved_locations();
	
	if (empty($locations)) {
		drupal_set_message(t('So far, you haven\'t set any locations to check weather at'), 'warning');
	}
	else {
		$form['saved_locations'] = array(
				'#type' => 'fieldset',
				'#title' => t('Saved locations'),
				'#description' => t('A list of locations you have saved to check wheather at'),
		);
		
		$form['saved_locations']['locations'] = array(
				'#type' => 'checkboxes',
				'#options' => $locations,
		);
		
		$form['submit'] = array(
				'#type' => 'submit',
				'#value' => t('Delete selected locations'),
				'#ajax' => array(
						'callback' => 'ajax_rb_weather_saved_locations_form_callback',
						'wrapper' => 'saved-locations-form',
						'effect' => 'fade',
						'method' => 'replace',
				),
		);
	}
	
	return $form;
}

/**
 * Implements hook_form_validate().
 * 
 * @param $form
 * @param $form_state
 */
function rb_weather_saved_locations_form_validate($form, &$form_state) {
	$selected_locations = extract_selected_locations($form_state);
	
	if (empty($selected_locations)) {
		form_set_error('locations', t('You have to select at least one checkbox if you want to remove a location'));
	}
}

/**
 * Implements hook_form_submit().
 * 
 * @param $form
 * @param $form_state
 */
function rb_weather_saved_locations_form_submit($form, &$form_state) {
	$selected_locations = extract_selected_locations($form_state);
	$deleted = 0;
	
	foreach ($selected_locations as $location) {
		$success = db_delete('rb_weather_locations')
						->condition('weather_location_identifier', $location, '=')
						->execute();
		
		if ($success) {
			$deleted++;
		}
	}

	if ($deleted) {
		drupal_set_message(format_plural($deleted,
				'1 location has successfully been removed', 
				'@count locations have successfully been removed',
				array('@count' => $deleted)));
	}
	
	$form_state['rebuild'] = TRUE;
}

function ajax_rb_weather_saved_locations_form_callback($form, &$form_state) {
	return $form;
}

/**
 * Returns an array containing the locations identifiers that have been selected.
 * 
 * @param $form_state
 *   The state of the form from which the selected locations will be extracted.
 */
function extract_selected_locations(&$form_state) {
	return array_diff($form_state['values']['locations'], array(0));
}

/**
 * Implements hook_form()
 * 
 * @param $form
 * @param $form_state
 */
function rb_weather_api_settings_form($form, &$form_state) {
	$locations_count = get_saved_locations_count();
	
	$form['api_key_fieldset'] = array(
			'#type' => 'fieldset',
			'#title' => t('Wunderground API key'),
			'#description' => t('In order to be able to pull data out of the wunderground API, you have to register for an API key'),
	);
	$form['api_key_fieldset']['api_key'] = array(
			'#type' => 'textfield',
			'#title' => t('API key'),
			'#description' => t('Enter you Wunderground API key here'),
			'#required' => TRUE,
	);
	
	if ($locations_count > 1) {
		$form['display_fieldset'] = array(
				'#type' => 'fieldset',
				'#title' => t('How many stations would you like to see'),
				'#description' => t('Select a number of weather stations you would like to see in your module')
		);
		$form['display_fieldset']['display_count'] = array(
				'#type' => 'select',
				'#title' => t('Stations count'),
				'#options' => drupal_map_assoc(range(1, get_saved_locations_count())),
		);
	}
	
	$form['submit'] = array(
			'#type' => 'submit',
			'#value' => t('Save settings'),
	);
	
	return $form;
}

/**
 * Implements hook_form_submit().
 * 
 * @param $form
 * @param $form_state
 */
function rb_weather_api_settings_form_submit($form, &$form_state) {
	global $user;
	
	$success = FALSE;
	
	if ( ! check_key_for_user_exists($user->uid)) {
		$success = db_insert('rb_weather_settings')
						->fields(array(
								'uid' => $user->uid,
								'api_key' => $form_state['values']['api_key'],
								'display' => $form_state['values']['display_count'],
						))
						->execute();
	}
	else {
		$success = db_update('rb_weather_settings')
						->fields(array(
								'api_key' => $form_state['values']['api_key'],
								'display' => $form_state['values']['display_count'],
						))
						->condition('uid', $user->uid, '=')
						->execute();
	}
				
	if ($success) {
		drupal_set_message(t('Your settings have been saved successfully'));
	} else {
		drupal_set_message(t('There was an error saving your settings', 'error'));
	}
	
	$form_state['rebuild'] = TRUE;
}

function check_key_for_user_exists($user_id) {
	return db_select('rb_weather_settings', 's')
				->fields('s', array('uid'))
				->condition('uid', $user_id, '=')
				->execute()
				->rowCount();
}