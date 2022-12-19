#!/usr/bin/env python3

import unittest
import subprocess

from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.wait import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC

target = "http://host.docker.internal:8080/"

class SimpleTest(unittest.TestCase):
    def setUp(self):
        opts = webdriver.ChromeOptions()
        self.driver = webdriver.Remote(
            command_executor="http://localhost:4444/wd/hub", options=opts
        )

        self.proc = subprocess.Popen([
            "./soro-server", "--port", "8080",
            "--resource_dir", "resources",
            "--server_resource_dir", "server_resources"
        ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        for line in self.proc.stderr:
            if b'tiles-server started on 0.0.0.0:' in line:
                break


    def test_simulation_button(self):
        self.driver.get(target)
        overlayToggle = self.driver.find_element(By.ID, "overlayToggleButton")
        overlayToggle.click()
        simButton = WebDriverWait(self.driver, 10).until(
            EC.element_to_be_clickable(
                (By.ID, "addInfrastructureComponentButton"))
        )
        simButton.click()
        divFilter = " and ".join([
            'contains(concat(" ", normalize-space(@class), " "), " lm_active ")',
            'contains(concat(" ", normalize-space(@class), " "), " lm_tab ")',
            '@title="Simulation"'
        ])
        xpath = f'//section/div[{divFilter}]'
        WebDriverWait(self.driver, 10).until(
            EC.presence_of_element_located((By.XPATH, xpath))
        )


    def tearDown(self):
        self.driver.close()
        self.driver.quit()
        self.proc.terminate()
        self.proc.wait()
        self.proc.stdout.close()
        self.proc.stderr.close()

if __name__ == "__main__":
    unittest.main()
