module Pinmap where

import System.IO (writeFile)
import System.Directory (createDirectoryIfMissing)
import Text.Read (readMaybe)
import Data.List (intercalate)
import Data.Maybe (mapMaybe)

-- d3 : (x,y);analog

data PinType = Analog | Digital | Power | Communication | Special
  deriving Show

data Pin = Pin
  { name    :: String
  , pinType :: PinType
  , x       :: Int
  , y       :: Int
  } deriving Show

-- removeSpaces :: String -> String
-- removeSpaces [] = []
-- removeSpaces (c:cs)
--   | c == ' ' = removeSpaces cs
--   | otherwise = c:removeSpaces cs

splitLine :: String->String->[String]
splitLine elim= go ""
  where
    go acc [] = [reverse acc]
    go acc (c:cs)
      | c `elem` elim = reverse acc:go "" cs
      | otherwise     = go (c:acc) cs


parsePin :: String -> Maybe Pin
parsePin s = case filter (not . null) (splitLine ":,();" (filter (/=' ') s)) of
             [name,xStr,yStr,typeStr] -> do
                px <- readMaybe xStr
                py <- readMaybe yStr
                pinType <- case typeStr of
                  "analog"        -> Just Analog
                  "digital"       -> Just Digital
                  "power"         -> Just Power
                  "communication" -> Just Communication
                  "special"       -> Just Special
                  _        -> Nothing
                return (Pin name pinType px py)
             _                -> Nothing


configPath :: String
configPath = "./pins.txt"

pinTypeToJSON :: PinType -> String
pinTypeToJSON Analog        = "analog"
pinTypeToJSON Digital       = "digital"
pinTypeToJSON Power         = "power"
pinTypeToJSON Communication = "communication"
pinTypeToJSON Special       = "special"

pinToJSON::Pin->String
pinToJSON p = "{\"name\":\"" ++ name p ++ "\",\"type\":\"" ++ pinTypeToJSON(pinType p) ++ "\",\"x\":" ++ show (x p) ++ ",\"y\":" ++ show (y p) ++ "}"

pinsToJSON::[Pin]->String
pinsToJSON ps = "[\n" ++ intercalate ",\n" (map pinToJSON ps) ++ "\n]"

main::IO ()
main = do 
          content <- readFile configPath
          let pins = mapMaybe parsePin (lines content)
          createDirectoryIfMissing True "GUI/gui/data"
          writeFile "GUI/gui/pins.json" (pinsToJSON pins)