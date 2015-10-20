<?php if ( ! empty($locations)) : ?>
<ul class="locations">
	<?php foreach($locations as $location): ?>
	<li>
		<h4>
			<span>
				<a href="<?php print $location->forecast_url; ?>"><?php print $location->display_location->full; ?></a>
			</span>
		</h4>
		<div class="observation clearfix">
			<div class="image">
				<img src="<?php print $location->icon_url; ?>" alt="<?php print $location->icon; ?>" />
			</div>
			<ul class="data">
				<li>
					<span><?php print t('Time:'); ?></span>
					<?php print date('d.m.Y H:i', $location->observation_epoch); ?>
					<?php print $location->local_tz_short; ?>
				</li>
				<li>
					<span><?php print t('Temperature'); ?></span>
					<?php print $location->temp_c; ?>
					<?php print t('°C'); ?>
				</li>
				<?php if ($location->wind_kph) : ?>
				<li>
					<span><?php print t('Wind:'); ?></span>
					<?php print $location->wind_kph; ?>
					<?php print t('km/h'); ?>
				</li>
				<?php endif; ?>
				<?php if ($location->wind_gust_kph) : ?>
				<li>
					<span><?php print t('Wind gust:'); ?></span>
					<?php print $location->wind_gust_kph; ?>
					<?php print t('km/h'); ?>
				</li>
				<?php endif; ?>
			</ul>
		</div>
	</li>
	<?php endforeach; ?>
</ul>
<?php else : ?>
<p><?php t('No locations to display'); ?></p>
<?php endif; ?>