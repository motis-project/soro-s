#!/usr/bin/env python3

import unittest
import subprocess

from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.wait import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC

target = "http://host.docker.internal:8080/"


class SeleniumTestCase(unittest.TestCase):
    """A base class for Selenium cases.

    Handles common set up and tear down.
    """

    def setUp(self):
        """Set up Selenium test resources.

        Creates the Selenium driver connection and soro server process
        """
        opts = webdriver.ChromeOptions()
        chromeDriver = webdriver.Remote(
            command_executor="http://localhost:4444/wd/hub",
            options=opts, keep_alive=True
        )

        opts = webdriver.FirefoxOptions()
        firefoxDriver = webdriver.Remote(
            command_executor="http://localhost:4444/wd/hub",
            options=opts, keep_alive=True
        )

        self.drivers = [chromeDriver, firefoxDriver]

        self.proc = subprocess.Popen([
            "./soro-server", "--port", "8080",
            "--resource_dir", "resources",
            "--server_resource_dir", "server_resources"
        ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        for line in self.proc.stderr:
            if b'tiles-server started on 0.0.0.0:' in line:
                break

    def tearDown(self):
        """Tear down Selenium test resources.

        Cleans up the Selenium driver connection and soro server process
        """
        for driver in self.drivers:
            driver.quit()

        self.proc.terminate()
        self.proc.wait()
        self.proc.stdout.close()
        self.proc.stderr.close()


class SimulationButtonTest(SeleniumTestCase):
    """Test case validating the simulation button."""

    def test_simulation_button(self):
        """Basic tast of the simulation button.

        Validates that clicking the button results in an active simulation tab.
        """
        for driver in self.drivers:
            with self.subTest(f"driver {driver.name}"):
                driver.get(target)

                overlayToggle = driver.find_element(
                    By.ID,
                    "overlayToggleButton"
                )
                overlayToggle.click()

                simButton = WebDriverWait(driver, 10).until(
                    EC.element_to_be_clickable(
                        (By.ID, "addInfrastructureComponentButton"))
                )
                simButton.click()
                normClass = 'concat(" ", normalize-space(@class), " ")'
                divFilter = " and ".join([
                    f'contains({normClass}, " lm_active ")',
                    f'contains({normClass}, " lm_tab ")',
                    '@title="Simulation"'
                ])
                xpath = f'//section/div[{divFilter}]'
                WebDriverWait(driver, 10).until(
                    EC.presence_of_element_located((By.XPATH, xpath))
                )


if __name__ == "__main__":
    unittest.main()
