<template>
	<div class="wrap-collapsible">
		<input
			:id="`collapsible-${inputId}`"
			class="collapsible-toggle hidden"
			type="checkbox"
		>
		<label
			:for="`collapsible-${inputId}`"
			class="collapsible-toggle"
		>{{ label }}</label>
		<div class="collapsible-content">
			<div
				id="stationRouteCollapsibleContent"
				class="content-inner"
			>
				<slot name="default" />
			</div>
		</div>
	</div>
</template>

<script>
export default {
	name: 'SoroCollapsible',

	props: {
		label: {
			type: String,
			required: true
		},
	},

	computed: {
		inputId() {
			return Math.random().toString();
		}
	}
};
</script>

<style scoped>
input[type=checkbox].hidden {
  opacity: 0;
}

.wrap-collapsible {
  display: inline;
  margin: 1.2em 0;
  width: 100%;
}

.collapsible-toggle {
  display: flex;
  flex-direction: row;
  max-height: 50%;
  font-weight: 550;
  font-family: var(--main-font-family);
  font-size: 16px;
  text-align: left;
  box-shadow: 0 10px 20px rgba(0, 0, 0, 0.19), 0 6px 6px rgba(0, 0, 0, 0.23);

  padding: 1em;

  margin: 1em;
  margin-bottom: 0;

  color: var(--text-color);
  background: var(--dialog-color);

  cursor: pointer;

  border-radius: var(--border-radius);
  transition: all 2.25s ease-out;
}

.collapsible-toggle::after {
  font-family: 'Material Icons';
  content: '\e5cc';

  vertical-align: middle;
  horiz-align: center;
  padding-left: calc(100% - 8.5em);
}

.collapsible-toggle:hover {
  background-color: var(--element-color);
}

.collapsible-content {
  position: absolute;
  width: calc(100% - 3.6em);
  max-height: 0;

  margin: 1em;
  margin-top: 0;
  overflow-y: auto;
  overflow-x: hidden;
  box-shadow: 0 20px 10px rgba(0, 0, 0, 0.19), 0 8px 6px rgba(0, 0, 0, 0.23);
}

.content-inner {
  display: flex;
  flex-direction: column;
  width: 95%;
  background: var(--dialog-color);
  border-bottom: 1px solid var(--dialog-color);

  border-bottom-left-radius: var(--border-radius);
  border-bottom-right-radius: var(--border-radius);
  padding: 1em
}

.collapsible-toggle:checked + .collapsible-toggle + .collapsible-content {
  max-height: 65%;
}

.collapsible-toggle:checked + .collapsible-toggle::after {
  content: '\e5ce';
}

.collapsible-toggle:checked + .collapsible-toggle {
  border-bottom-right-radius: 0;
  border-bottom-left-radius: 0;
}
</style>