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
		))
		->execute();
		
		drupal_set_message(t('A new weather location has successfully been added'));
		
		//TODO Dorobit resetovanie formularu po uspesnom odoslani
		$form_state['can_reset'] = TRUE;
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
		
		foreach ($locations as $id => $location) {
			$form['saved_locations'][] = array(
					'#type' => 'checkbox',
					'#title' => $location,
					'#attributes' => array(
							'name' => 'saved_locations[]',
					),
					'#return_value' => $id,
			);
		}
		
		$form['submit'] = array(
				'#type' => 'submit',
				'#value' => t('Delete selected locations'),
				/*'#ajax' => array(
						'callback' => 'ajax_rb_weather_saved_locations_form_callback',
						'wrapper' => 'saved-locations-form',
						'method' => 'replace',
						'effect' => 'fade',
				),*/
		);
	}
	
	return $form;
}

function rb_weather_saved_locations_form_validate($form, &$form_state) {
	print_r($form_state['values']);
	
	if (empty($form_state['values']['saved_locations'])) {
		form_set_error('saved_locations', t('You have to select at least one of the locations for a successful removal'));
	}
}

function rb_weather_saved_locations_form_submit($form, &$form_state) {
	$form_state['rebuild'] = TRUE;
}

function ajax_rb_weather_saved_locations_form_callback($form, &$form_state) {
	return $form;
}